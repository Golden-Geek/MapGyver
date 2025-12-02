/*
  ==============================================================================

	VideoMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/MGEngine.h"
#include "VideoMedia.h"

void* get_proc_address(void* ctx, const char* name) {

#if JUCE_WINDOWS
	static HMODULE glModule = GetModuleHandleA("opengl32.dll");

	// 1. Get the pointer to wglGetProcAddress itself
	using wglProc = void* (__stdcall*)(const char*);
	static wglProc wgl_get_proc_address = (wglProc)GetProcAddress(glModule, "wglGetProcAddress");

	// 2. Try to use it
	void* p = nullptr;
	if (wgl_get_proc_address) {
		p = wgl_get_proc_address(name);
	}

	// 3. Fallback for standard functions
	if (p == nullptr || p == (void*)0x1 || p == (void*)0x2 || p == (void*)0x3 || p == (void*)-1) {
		p = (void*)GetProcAddress(glModule, name);
	}

	return p;
#endif
}

void mpv_update(void* ctx)
{
	((VideoMedia*)ctx)->onMPVUpdate();
}

void mpv_wakeup(void* ctx)
{
	((VideoMedia*)ctx)->onMPVWakeup();
}

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


	//Init MPV
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
	// Allow MPV to be slightly imprecise with audio to get instant video response
	mpv_set_option_string(mpv, "hr-seek", "no");
	// Optional: If you are scrubbing, prevent MPV from waiting for audio to catch up
	mpv_set_option_string(mpv, "hr-seek-framedrop", "yes");
	int result = mpv_initialize(mpv);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR(niceName, "could not initialize mpv context : " << result << " (" << mpv_error_string(result) << ")");
		return;
	}

	// Request hw decoding, just for testing.
	mpv_set_option_string(mpv, "hwdec", "auto");

	//register events

	mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
	mpv_set_wakeup_callback(mpv, mpv_wakeup, this);

}

VideoMedia::~VideoMedia()
{


	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->removeAudioManagerListener(this);

	AudioManager::getInstance()->graph.removeNode(audioNodeID);
}

void VideoMedia::clearItem()
{
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
			// Update speed
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
	// Existing audio setup logic (mostly for VLC / Graph connections)
	AudioManager::getInstance()->graph.disconnectNode(audioNodeID);

	if (isCurrentlyLoadingData || isClearing || Engine::mainEngine->isClearing) return;

	//retrieve track info from mpv 


	mpv_node result;
	if (mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &result) == MPV_ERROR_SUCCESS)
	{
		if (result.format == MPV_FORMAT_NODE_ARRAY)
		{
			mpv_node_list* list = result.u.list;
			for (int i = 0; i < list->num; i++)
			{
				if (list->values[i].format == MPV_FORMAT_NODE_MAP)
				{
					mpv_node_list* track_props = list->values[i].u.list;
					const char* type = nullptr;
					long long channels = 0;

					for (int j = 0; j < track_props->num; j++)
					{
						if (strcmp(track_props->keys[j], "type") == 0 && track_props->values[j].format == MPV_FORMAT_STRING)
						{
							type = track_props->values[j].u.string;
						}
						if (strcmp(track_props->keys[j], "demux-channels") == 0 && track_props->values[j].format == MPV_FORMAT_INT64)
						{
							channels = track_props->values[j].u.int64;
						}
					}

					if (type && strcmp(type, "audio") == 0 && channels > 0)
					{
						int sampleRate = AudioManager::getInstance()->graph.getSampleRate();
						int bufferSize = AudioManager::getInstance()->graph.getBlockSize();
						int numChannels = (int)channels;

						audioProcessor->setPlayConfigDetails(0, numChannels, sampleRate, bufferSize);
						audioProcessor->prepareToPlay(sampleRate, bufferSize);

						int minChannels = jmin(numChannels, AudioManager::getInstance()->getNumUserOutputs());
						for (int ch = 0; ch < minChannels; ++ch)
						{
							if (AudioManager::getInstance()->graph.canConnect({ { audioNodeID, ch }, { AUDIO_OUTPUTMIXER_GRAPH_ID, ch } }))
							{
								AudioManager::getInstance()->graph.addConnection({ { audioNodeID, ch }, { AUDIO_OUTPUTMIXER_GRAPH_ID, ch } });
							}
						}
						// Found and configured the first audio track, so we can stop.
						break;
					}
				}
			}
		}
		mpv_free_node_contents(&result);
	}
}

void VideoMedia::audioSetupChanged()
{
	setupAudio();
}

void VideoMedia::load()
{
	// --- Get Path ---
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
	// 2. Load File


	startTimerHz(50);
	NLOG(niceName, "Loading video: " << path);


	const char* cmd[] = { "loadfile", path.toRawUTF8(), NULL };
	mpv_command_async(mpv, 0, cmd);

	int is_paused = 0;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &is_paused);

	state->setValueWithData(IDLE); // Will switch to PLAYING/READY when MPV event arrives

	// Reset Length/Position (will be updated via timer)
	length->setValue(0);
	position->setValue(0);

	if (playAtLoad->boolValue()) play();


	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
}


void VideoMedia::initGLInternal()
{
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
	// Use MPV to render directly to the current OpenGL FBO
	// Note: We need to flip Y because JUCE and MPV might disagree on coords depending on setup

	Init2DViewport(frameBuffer.getWidth(), frameBuffer.getHeight());
	glClearColor(.5f, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	int frameBufferToPasstoMPV = static_cast<int>(frameBuffer.getFrameBufferID()); //pass our framebuffer
	mpv_opengl_fbo mpfbo{
		frameBufferToPasstoMPV,
		frameBuffer.getWidth(),
		frameBuffer.getHeight(),
		0 // internal format unknown
	};

	int flip_y = 1;
	mpv_render_param params[] = {
		// {MPV_RENDER_PARAM_FLIP_Y, &flip_y}, // Optional
		{MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
		{MPV_RENDER_PARAM_FLIP_Y, &flip_y},
		{ MPV_RENDER_PARAM_INVALID, nullptr }
	};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// 2. Ensure MPV draws to the whole FBO (JUCE might have clipped the region)
	glDisable(GL_SCISSOR_TEST);

	// 3. Ensure the video is opaque (Disable blending so it overwrites the background)
	glDisable(GL_BLEND);

	int result = mpv_render_context_render(mpv_gl, params);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR(niceName, "mpv rendering error : " << result << " (" << mpv_error_string(result) << ")");
	}
}

void VideoMedia::closeGLInternal()
{
	if (mpv_gl)
		mpv_render_context_free(mpv_gl);

	if (mpv)
	{
		mpv_terminate_destroy(mpv);
	}

	mpv = nullptr;
	mpv_gl = nullptr;
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

	mpv_event* e = mpv_wait_event(mpv, 0);
	while (e->event_id != MPV_EVENT_NONE && e->event_id != MPV_EVENT_IDLE)
	{

		switch (e->event_id)
		{
		case MPV_EVENT_FILE_LOADED:
		{
			videoWidth = getMPVIntProperty("width");
			videoHeight = getMPVIntProperty("height");
			length->setValue(getMPVDoubleProperty("duration"));
			position->setRange(0, length->doubleValue());

			NLOG(niceName, "Video loaded: " << videoWidth << "x" << videoHeight << ", Length: " << length->doubleValue());

			state->setValueWithData(READY);
			//pause here
			pause();

			mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
		}
		break;

		case MPV_EVENT_PROPERTY_CHANGE:
		{
			mpv_event_property* prop = (mpv_event_property*)e->data;

			String propName = String(prop->name);
			//NLOG(niceName, "MPV Property Change Event : " << propName);

			if (propName == "duration")
			{
				if (prop->format == MPV_FORMAT_DOUBLE)
				{
					double dur = *(double*)prop->data;
					length->setValue(dur);
					mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_LENGTH_CHANGED, this));
				}
			}
			else if (propName == "time-pos")
			{
				if (prop->format == MPV_FORMAT_DOUBLE)
				{
					double pos = *(double*)prop->data;
					updatingPosFromVLC = true;
					position->setValue(pos);
					updatingPosFromVLC = false;
				}
			}
		}
		break;

		case MPV_EVENT_START_FILE:
		{
			//NLOG(niceName, "MPV Start File Event");
		}
		break;

		case MPV_EVENT_END_FILE:
		{
			//NLOG(niceName, "MPV End File Event");
			if (loop->boolValue())
			{
				seek(0);
				play();
			}
			else
			{
				state->setValueWithData(IDLE);
			}
		}
		break;

		case MPV_EVENT_COMMAND_REPLY:
		{
			// Handle command replies if needed
			//NLOG(niceName, "MPV Command Reply Event: ");
		}
		break;

		default:
			//NLOG(niceName, "MPV Unhandled Event: " << String(mpv_event_name(e->event_id)) << "(" << e->event_id << ")");
			break;
		}

		e = mpv_wait_event(mpv, 0);

	}
}

int VideoMedia::getMPVIntProperty(const char* name)
{
	int result{};
	mpv_get_property(mpv, name, MPV_FORMAT_INT64, &result);
	return result;
}

inline double VideoMedia::getMPVDoubleProperty(const char* name)
{
	double result{};
	mpv_get_property(mpv, name, MPV_FORMAT_DOUBLE, &result);
	return result;
}

inline String VideoMedia::getMPVStringProperty(const char* name)
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
	//// "absolute" seeking
	//NLOG(niceName, "Seek to " << target);
	//// 2. Set the property directly
	//// This is equivalent to "seek <target> absolute"
	//int result = mpv_set_property_async(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE, &target);

	//if (result < 0) {
	//	NLOGERROR(niceName, "Seek failed: " << mpv_error_string(result));
	//}
	//position->setValue(target);


	mpv_node args[3];

	// Arg 0: Command
	args[0].format = MPV_FORMAT_STRING;
	args[0].u.string = "seek";

	// Arg 1: Time (Pass the number directly!)
	args[1].format = MPV_FORMAT_DOUBLE;
	args[1].u.double_ = target;

	// Arg 2: Flags
	args[2].format = MPV_FORMAT_STRING;
	args[2].u.string = "absolute";

	mpv_node command;
	mpv_node_list list{ 3, args };
	command.format = MPV_FORMAT_NODE_ARRAY;
	command.u.list = &list;

	int result = mpv_command_node_async(mpv, 0, &command);
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

