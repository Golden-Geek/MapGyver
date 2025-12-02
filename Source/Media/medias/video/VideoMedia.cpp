/*
  ==============================================================================

	VideoMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/MGEngine.h"

// ==============================================================================
// HELPER: Windows OpenGL Loading
// ==============================================================================
#if JUCE_WINDOWS
#include <windows.h>
#endif

void* get_proc_address(void* ctx, const char* name) {
#if JUCE_WINDOWS
	static HMODULE glModule = GetModuleHandleA("opengl32.dll");
	using wglProc = void* (__stdcall*)(const char*);
	static wglProc wgl_get_proc_address = (wglProc)GetProcAddress(glModule, "wglGetProcAddress");

	void* p = nullptr;
	if (wgl_get_proc_address) p = wgl_get_proc_address(name);
	if (p == nullptr || p == (void*)0x1 || p == (void*)0x2 || p == (void*)0x3 || p == (void*)-1) {
		p = (void*)GetProcAddress(glModule, name);
	}
	return p;
#else
	return nullptr;
#endif
}

void mpv_update(void* ctx) { ((VideoMedia*)ctx)->onMPVUpdate(); }
void mpv_wakeup(void* ctx) { ((VideoMedia*)ctx)->onMPVWakeup(); }


// ==============================================================================
// VIDEO MEDIA IMPLEMENTATION
// ==============================================================================

VideoMedia::VideoMedia(var params) :
	Media(getTypeString(), params),
	controlsCC("Controls"),
	audioCC("Audio"),
	updatingPosFromVLC(false),
	manuallySeeking(false),
	timeAtLastSeek(0),
	audioProcessor(nullptr)
{
	// --- Parameters ---
	source = addEnumParameter("Source", "Source");
	source->addOption("File", Source_File)->addOption("URL", Source_URL);

	filePath = addFileParameter("File path", "File path", "");
	filePath->setAutoReload(true);

	url = addStringParameter("URL", "URL", "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", false);

	state = addEnumParameter("State", "Player state");
	for (int i = 0; i < STATES_MAX; i++) state->addOption(playerStateNames[i], (PlayerState)i);
	state->setControllableFeedbackOnly(true);

	position = addFloatParameter("Time", "Time of video", 0, 0, 10);
	length = addFloatParameter("Length", "Length of video", 10, 0);
	length->setControllableFeedbackOnly(true);

	// --- Controls ---
	playTrigger = controlsCC.addTrigger("Play", "Play video");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop video");
	pauseTrigger = controlsCC.addTrigger("Pause", "Pause video");
	restartTrigger = controlsCC.addTrigger("Restart", "Restart video");
	playAtLoad = controlsCC.addBoolParameter("Play at load", "Play as soon as the video is loaded", false);
	loop = controlsCC.addBoolParameter("Loop", "Loop video", false);
	playSpeed = controlsCC.addFloatParameter("Speed", "Speed of video", 1, 0);
	volume = audioCC.addFloatParameter("Volume", "Volume of video", 1, 0, 1);

	addChildControllableContainer(&controlsCC);
	addChildControllableContainer(&audioCC);

	// --- Audio Processor Setup ---
	audioNodeID = AudioProcessorGraph::NodeID(AudioManager::getInstance()->getUniqueNodeGraphID());
	std::unique_ptr<VideoMediaAudioProcessor> ap(new VideoMediaAudioProcessor(this));
	audioProcessor = ap.get();
	AudioManager::getInstance()->graph.addNode(std::move(ap), audioNodeID);
	AudioManager::getInstance()->addAudioManagerListener(this);

	customFPSTick = true;

	// --- MPV & Audio Pipe Init ---

	// 1. Setup Unique Pipe Name
	Uuid uuid;
	uniquePipePath = "\\\\.\\pipe\\mpv_audio_" + uuid.toString();

	// 2. Start Pipe Thread (Must happen before MPV connects)
	pipeThread.reset(new PipeThread(this, uniquePipePath));

	// 3. Init MPV
	mpv = mpv_create();
	if (!mpv)
	{
		NLOGERROR(niceName, "Could not create mpv context");
		return;
	}

	mpv_set_option_string(mpv, "terminal", "yes");
	mpv_set_option_string(mpv, "msg-level", "all=v");
	mpv_set_option_string(mpv, "vo", "libmpv");
	mpv_set_option_string(mpv, "force-window", "yes");
	mpv_set_option_string(mpv, "hr-seek", "no");
	mpv_set_option_string(mpv, "hr-seek-framedrop", "yes");
	mpv_set_option_string(mpv, "hwdec", "auto");

	// --- AUDIO ROUTING CONFIGURATION ---
	double sampleRate = AudioManager::getInstance()->graph.getSampleRate();
	if (sampleRate <= 0) sampleRate = 48000.0; // Fallback

	mpv_set_option_string(mpv, "audio-samplerate", String(sampleRate).toRawUTF8());
	mpv_set_option_string(mpv, "audio-resample", "yes");

	mpv_set_option_string(mpv, "ao", "pcm");
	mpv_set_option_string(mpv, "ao-pcm-format", "f32le"); // Float 32 matches AudioBuffer
	mpv_set_option_string(mpv, "ao-pcm-channels", "2");   // Stereo
	mpv_set_option_string(mpv, "ao-pcm-rate", String(sampleRate).toRawUTF8());
	mpv_set_option_string(mpv, "ao-pcm-file", uniquePipePath.toRawUTF8());

	int result = mpv_initialize(mpv);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR(niceName, "could not initialize mpv context : " << result << " (" << mpv_error_string(result) << ")");
		return;
	}

	mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
	mpv_set_wakeup_callback(mpv, mpv_wakeup, this);
}

VideoMedia::~VideoMedia()
{
	// Cleanup MPV
	closeGLInternal(); // Destroys mpv and mpv_gl

	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->removeAudioManagerListener(this);

	AudioManager::getInstance()->graph.removeNode(audioNodeID);
}

void VideoMedia::clearItem()
{
	//clear mpv
	stop();
	
	// Cleanup Pipe Thread
	if (pipeThread) {
		pipeThread->shutdown();
		pipeThread.reset();
	}


	Media::clearItem();
}

void VideoMedia::onContainerParameterChanged(Parameter* p)
{
	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
	}
	else if (p == source || p == filePath || p == url)
	{
		load();
	}
	else if (p == position)
	{
		if (!updatingPosFromVLC)
			seek(position->doubleValue());
	}
}


void VideoMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	Media::onControllableFeedbackUpdateInternal(cc, c);

	if (cc == &controlsCC)
	{
		if (c == playTrigger) play();
		else if (c == stopTrigger) stop();
		else if (c == pauseTrigger) pause();
		else if (c == restartTrigger) restart();
		else if (c == playSpeed)
		{
			double speed = playSpeed->doubleValue();
			mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &speed);
		}
	}
	else if (cc == &audioCC)
	{
		if (c == volume)
		{
			double vol = volume->doubleValue() * 100.0;
			mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
		}
	}
}

void VideoMedia::setupAudio()
{
	// Simply prepare the processor. MPV pipes audio automatically based on init options.
	// We don't need to check track lists anymore because we force ao=pcm in constructor.

	int sampleRate = AudioManager::getInstance()->graph.getSampleRate();
	int bufferSize = AudioManager::getInstance()->graph.getBlockSize();
	int numChannels = 2; // We enforced 2 channels in MPV options

	// Prepare the processor
	audioProcessor->setPlayConfigDetails(0, numChannels, sampleRate, bufferSize);
	audioProcessor->prepareToPlay(sampleRate, bufferSize);

	// Connect to Output Mixer
	int minChannels = jmin(numChannels, AudioManager::getInstance()->getNumUserOutputs());
	for (int ch = 0; ch < minChannels; ++ch)
	{
		if (AudioManager::getInstance()->graph.canConnect({ { audioNodeID, ch }, { AUDIO_OUTPUTMIXER_GRAPH_ID, ch } }))
		{
			AudioManager::getInstance()->graph.addConnection({ { audioNodeID, ch }, { AUDIO_OUTPUTMIXER_GRAPH_ID, ch } });
		}
	}
}

void VideoMedia::audioSetupChanged()
{
	setupAudio();
}

void VideoMedia::load()
{
	String path;
	if (source->getValueDataAsEnum<VideoSource>() == Source_File)
	{
		File f = filePath->getFile();
		if (!f.existsAsFile())
		{
			NLOGWARNING(niceName, "File not found : " << f.getFullPathName());
			state->setValueWithData(UNLOADED);
			return;
		}
		path = f.getFullPathName();
	}
	else
	{
		path = url->stringValue();
	}

	jassert(mpv != nullptr);
	stopTimer();

	if (mpv_gl == nullptr)
	{
		NLOGWARNING(niceName, "Not loading now, needs GL to be initialized");
		return;
	}

	startTimerHz(50);
	NLOG(niceName, "Loading video: " << path);

	const char* cmd[] = { "loadfile", path.toRawUTF8(), NULL };
	mpv_command_async(mpv, 0, cmd);

	int is_paused = 0;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &is_paused);

	state->setValueWithData(IDLE);
	length->setValue(0);
	position->setValue(0);

	if (playAtLoad->boolValue()) play();

	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
}


void VideoMedia::initGLInternal()
{
	// ... (Your GL Init code from previous implementation) ...
	// Ensure you are using the GL PixelStore fixes I mentioned earlier

	mpv_opengl_init_params gl_init_params[1] = { get_proc_address, nullptr };
	mpv_render_param params[]{
		{MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
		{MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
		{MPV_RENDER_PARAM_INVALID, nullptr}
	};

	int result = mpv_render_context_create(&mpv_gl, mpv, params);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR(niceName, "failed to initialize mpv GL context : " << result << " (" << mpv_error_string(result) << ")");
		return;
	}

	mpv_render_context_set_update_callback(mpv_gl, mpv_update, this);

	LOG("MPV GL created");
	load();
}

void VideoMedia::renderGLInternal()
{
	Init2DViewport(frameBuffer.getWidth(), frameBuffer.getHeight());
	glClearColor(.5f, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	int frameBufferToPasstoMPV = static_cast<int>(frameBuffer.getFrameBufferID());
	mpv_opengl_fbo mpfbo{
		frameBufferToPasstoMPV,
		frameBuffer.getWidth(),
		frameBuffer.getHeight(),
		0x8058 // Internal format RGBA8
	};

	int flip_y = 1;
	mpv_render_param params[] = {
		{MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
		{MPV_RENDER_PARAM_FLIP_Y, &flip_y},
		{MPV_RENDER_PARAM_INVALID, nullptr}
	};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // FIX ALIGNMENT
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);

	int result = mpv_render_context_render(mpv_gl, params);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR(niceName, "mpv rendering error : " << result << " (" << mpv_error_string(result) << ")");
	}
}

void VideoMedia::closeGLInternal()
{
	if (mpv_gl) {
		mpv_render_context_free(mpv_gl);
		mpv_gl = nullptr;
	}

	// NOTE: We do not destroy `mpv` here anymore if we want to keep audio running 
	// independently of GL, but typically in your structure closeGL implies shutdown.
	// If you destroy mpv here, ensure pipeThread is handled too if you plan to reload.
	// For now, let's assume VideoMedia destruction handles the final mpv cleanup.
}

void VideoMedia::onMPVUpdate()
{
	shouldRedraw = true;
}

void VideoMedia::onMPVWakeup()
{
}

void VideoMedia::pullEvents()
{
	if (mpv == nullptr) return;

	while (true)
	{
		mpv_event* e = mpv_wait_event(mpv, 0);
		if (e->event_id == MPV_EVENT_NONE) break;

		switch (e->event_id)
		{
		case MPV_EVENT_FILE_LOADED:
			videoWidth = getMPVIntProperty("width");
			videoHeight = getMPVIntProperty("height");
			length->setValue(getMPVDoubleProperty("duration"));
			position->setRange(0, length->doubleValue());

			//check actual number of tracks
			numAudioTracks = getMPVIntProperty("audio-ids/count");
			LOG("Num audio tracks : " << numAudioTracks);

			state->setValueWithData(READY);
			pause();
			mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));

			setupAudio();
			break;

		case MPV_EVENT_END_FILE:
			if (loop->boolValue()) {
				seek(0);
				play();
			}
			else {
				state->setValueWithData(IDLE);
			}
			break;

		case MPV_EVENT_PROPERTY_CHANGE:
		{
			mpv_event_property* prop = (mpv_event_property*)e->data;
			if (String(prop->name) == "time-pos" && prop->format == MPV_FORMAT_DOUBLE) {
				updatingPosFromVLC = true;
				position->setValue(*(double*)prop->data);
				updatingPosFromVLC = false;
			}
		}
		break;
		}
	}
}

int VideoMedia::getMPVIntProperty(const char* name)
{
	int64_t result = 0;
	mpv_get_property(mpv, name, MPV_FORMAT_INT64, &result);
	return (int)result;
}

double VideoMedia::getMPVDoubleProperty(const char* name)
{
	double result = 0;
	mpv_get_property(mpv, name, MPV_FORMAT_DOUBLE, &result);
	return result;
}

String VideoMedia::getMPVStringProperty(const char* name)
{
	char* result = nullptr;
	mpv_get_property(mpv, name, MPV_FORMAT_STRING, &result);
	String strResult = String(result);
	mpv_free(result);
	return strResult;
}

// CONTROL
void VideoMedia::play() {
	int paused = 0;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
	state->setValueWithData(PLAYING);
}

void VideoMedia::stop() {
	pause();
	seek(0);
	state->setValueWithData(READY);
}

void VideoMedia::pause() {
	int paused = 1;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
	state->setValueWithData(PAUSED);
}

void VideoMedia::restart() {
	seek(0);
	play();
}

void VideoMedia::seek(double time)
{
	double target = jlimit(0.0, length->doubleValue(), time);

	mpv_node args[3];
	args[0].format = MPV_FORMAT_STRING; args[0].u.string = "seek";
	args[1].format = MPV_FORMAT_DOUBLE; args[1].u.double_ = target;
	args[2].format = MPV_FORMAT_STRING; args[2].u.string = "absolute";

	mpv_node command;
	mpv_node_list list{ 3, args };
	command.format = MPV_FORMAT_NODE_ARRAY;
	command.u.list = &list;

	mpv_command_node_async(mpv, 0, &command);
}

// =========================================================================================
// STANDARD MEDIA HANDLERS
// =========================================================================================

void VideoMedia::handleEnter(double time, bool doPlay)
{
	Media::handleEnter(time, doPlay);
	seek(time);
	if (doPlay) play();
}

void VideoMedia::handleExit()
{
	Media::handleExit();
	stop();
}

void VideoMedia::handleSeek(double time)
{
	Media::handleSeek(time);
	seek(time);
}

void VideoMedia::handleStop()
{
	pause();
}

void VideoMedia::handleStart()
{
	play();
}

bool VideoMedia::isPlaying()
{
	return state->getValueDataAsEnum<PlayerState>() == PLAYING;
}

double VideoMedia::getMediaLength()
{
	return length->doubleValue();
}

Point<int> VideoMedia::getMediaSize(const String& texName)
{
	return { videoWidth, videoHeight };
}

String VideoMedia::getMediaContentName() const
{
	if (filePath != nullptr && source->getValueDataAsEnum<VideoSource>() == Source_File)
		return filePath->getFile().getFileNameWithoutExtension();
	else if (url != nullptr && source->getValueDataAsEnum<VideoSource>() == Source_URL)
		return url->stringValue().substring(url->stringValue().lastIndexOf("/") + 1);

	return Media::getMediaContentName();
}

void VideoMedia::afterLoadJSONDataInternal()
{
	Media::afterLoadJSONDataInternal();
	setupAudio();
	if (state->getValueDataAsEnum<PlayerState>() == PLAYING) play();
}

void VideoMedia::timerCallback()
{
	pullEvents();
}


// PIPE THREAD

VideoMedia::PipeThread::PipeThread(VideoMedia* owner, String path)
	: Thread("MPV Audio Pipe"), owner(owner), pipePath(path)
{
	// 1. Create the Named Pipe immediately so it exists when MPV tries to open it
	pipeHandle = CreateNamedPipeA(
		pipePath.toRawUTF8(),
		PIPE_ACCESS_INBOUND,        // Read only
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1,                          // Max instances (1)
		65536, 65536,               // Buffer sizes (64k)
		0,
		NULL
	);

	readBuffer.resize(4096); // 4k buffer for reading chunks
	startThread();
}

VideoMedia::PipeThread::~PipeThread()
{
	shutdown();
}

void VideoMedia::PipeThread::shutdown()
{
	stopThread(1000);
	if (pipeHandle != INVALID_HANDLE_VALUE) {
		DisconnectNamedPipe(pipeHandle);
		CloseHandle(pipeHandle);
		pipeHandle = INVALID_HANDLE_VALUE;
	}
}

void VideoMedia::PipeThread::run()
{
	if (pipeHandle == INVALID_HANDLE_VALUE) return;

	// Wait for MPV to connect
	bool connected = ConnectNamedPipe(pipeHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	if (connected)
	{
		DWORD bytesAvail = 0;
		while (!threadShouldExit() && !owner->isClearing)
		{
			if (PeekNamedPipe(pipeHandle, NULL, 0, NULL, &bytesAvail, NULL))
			{
				if (bytesAvail > 0)
				{
					DWORD bytesRead = 0;
					// Limit read size to our buffer size
					DWORD bytesToRead = jmin((DWORD)(readBuffer.size() * sizeof(float)), bytesAvail);

					// Now we know ReadFile won't block because data is guaranteed to be there
					if (ReadFile(pipeHandle, readBuffer.data(), bytesToRead, &bytesRead, NULL) && bytesRead > 0)
					{
						if (owner && owner->audioProcessor)
						{
							int numFloats = bytesRead / sizeof(float);
							int numChannels = 2; 
							int numFrames = numFloats / numChannels;

							owner->audioProcessor->onAudioPlay(readBuffer.data(), numFrames, 0); 
						}
					}
				}
				else
				{
					// Pipe is connected but no audio is flowing (Paused/Stopped).
					// Sleep briefly to let the thread loop check 'threadShouldExit()'
					wait(5);
				}
			}
			else
			{
				// Peek failed (Pipe disconnected or broken). 
				// Sleep briefly to avoid 100% CPU usage loop.
				wait(5);
			}
		}
	}
}
