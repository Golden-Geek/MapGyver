/*
  ==============================================================================

	MPVPlayer.cpp
	Created: 3 Dec 2025 9:44:00am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

namespace {
	constexpr uint64_t kReqDuration = 1;
	constexpr uint64_t kReqWidth = 2;
	constexpr uint64_t kReqHeight = 3;
	constexpr uint64_t kReqChannels = 4;

	constexpr int kMaskDuration = 1 << 0;
	constexpr int kMaskWidth = 1 << 1;
	constexpr int kMaskHeight = 1 << 2;
	constexpr int kMaskChannels = 1 << 3;
	constexpr int kMaskAll = kMaskDuration | kMaskWidth | kMaskHeight | kMaskChannels;
}

// ==============================================================================
// HELPER: Windows OpenGL Loading
// ==============================================================================
void* get_gl_proc_address(void* ctx, const char* name) {

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


void mpv_update(void* ctx) { ((MPVPlayer*)ctx)->onMPVUpdate(); }
void mpv_wakeup(void* ctx) { ((MPVPlayer*)ctx)->onMPVWakeup(); }
// ==============================================================================


MPVPlayer::MPVPlayer(const String& filePath) :
	filePath(filePath)
{


	// 1. Setup Unique Pipe Name
	Uuid uuid;
	uniquePipePath = "\\\\.\\pipe\\mpv_audio_" + uuid.toString();

	// 2. Start Pipe Thread (Must happen before MPV connects)
	pipeThread.reset(new AudioPipeThread(this, uniquePipePath));

	setupMPV();

	MPVTimers::getInstance()->registerMPV(this);
}

MPVPlayer::~MPVPlayer()
{
	clear();
	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->removeAudioManagerListener(this);
}

void MPVPlayer::clear()
{
	if (MPVTimers::getInstanceWithoutCreating())
		MPVTimers::getInstance()->unregisterMPV(this);

	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->graph.removeNode(audioNodeID);

	// Cleanup Pipe Thread
	if (pipeThread) {
		pipeThread->shutdown();
		pipeThread.reset();
	}

	if (mpv_gl)
	{
		mpv_render_context_free(mpv_gl);
		mpv_gl = nullptr;
	}

	if (mpv)
	{
		mpv_terminate_destroy(mpv);
		mpv = nullptr;
	}
}

void MPVPlayer::setupMPV()
{
	mpv = mpv_create();
	if (!mpv)
	{
		NLOGERROR("MPV Player", "Could not create mpv context");
		return;
	}

	mpv_set_option_string(mpv, "terminal", "yes");
	mpv_set_option_string(mpv, "msg-level", "all=v");
	mpv_set_option_string(mpv, "vo", "libmpv");
	mpv_set_option_string(mpv, "force-window", "yes");
	mpv_set_option_string(mpv, "hr-seek", "yes");
	mpv_set_option_string(mpv, "hr-seek-framedrop", "yes");

	// "auto" is good, but sometimes picks copy-back methods.
	// "nvdec" (Nvidia) or "vaapi" (Intel/Linux) are strictly keep-in-VRAM.
	// "auto-safe" defaults to the best available hardware method.
	mpv_set_option_string(mpv, "hwdec", "auto-safe");

	// 2. GPU CONTEXT
	// Explicitly tell MPV we are in an OpenGL environment so it attempts 
	// to map the HW surface directly to a GL Texture.
	mpv_set_option_string(mpv, "gpu-api", "opengl");

#if JUCE_WINDOWS
	mpv_set_option_string(mpv, "gpu-context", "wgl"); // Use "cocoa" on Mac, "x11" on Linux
#endif

	// 3. DIRECT RENDERING (Crucial for CPU offload)
	// Allows the decoder to write directly into video memory allocated by the renderer
	mpv_set_option_string(mpv, "vd-lavc-dr", "yes");

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
		NLOGERROR("MPV Player", "could not initialize mpv context : " << result << " (" << mpv_error_string(result) << ")");
		return;
	}

	//mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
	//mpv_observe_property(mpv, 0, "audio-params/channel-count", MPV_FORMAT_INT64);
	//mpv_observe_property(mpv, 0, "width", MPV_FORMAT_INT64);
	//mpv_observe_property(mpv, 0, "height", MPV_FORMAT_INT64);
	//mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);
	mpv_set_wakeup_callback(mpv, mpv_wakeup, this);


}

void MPVPlayer::loadFile()
{
	if (mpv_gl == nullptr)
	{
		NLOGWARNING("MPV Player", "Not loading now, needs GL to be initialized");
		return;
	}


	const char* cmd[] = { "loadfile", filePath.toRawUTF8(), NULL };
	mpv_command_async(mpv, 0, cmd);
}

void MPVPlayer::setupGL()
{
	mpv_opengl_init_params gl_init_params[1] = { get_gl_proc_address, nullptr };
	mpv_render_param params[]{
		{MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
		{MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
		{MPV_RENDER_PARAM_INVALID, nullptr}
	};

	int result = mpv_render_context_create(&mpv_gl, mpv, params);

	if (result != MPV_ERROR_SUCCESS)
	{
		NLOGERROR("MPV Player", "failed to initialize mpv GL context : " << result << " (" << mpv_error_string(result) << ")");
		return;
	}

	mpv_render_context_set_update_callback(mpv_gl, mpv_update, this);

	loadFile();
}

void MPVPlayer::renderGL(OpenGLFrameBuffer* frameBuffer)
{
	if (mpv_gl == nullptr)
	{
		setupGL();
		return;
	}

	if (!fileInfo.fileLoaded)
	{
		return;
	}

	int frameBufferToPasstoMPV = static_cast<int>(frameBuffer->getFrameBufferID());
	mpv_opengl_fbo mpfbo{
		frameBufferToPasstoMPV,
		frameBuffer->getWidth(),
		frameBuffer->getHeight(),
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
		NLOGERROR("MPV Player", "mpv rendering error : " << result << " (" << mpv_error_string(result) << ")");
	}
}

bool MPVPlayer::isGLInit()
{
	return mpv_gl != nullptr;
}


void MPVPlayer::play()
{
	if (mpv == nullptr) return;
	int paused = 0;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

void MPVPlayer::pause()
{
	if (mpv == nullptr) return;
	int paused = 1;
	mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

void MPVPlayer::stop()
{
	pause();
	setPosition(0.0);
}

void MPVPlayer::setPosition(double pos)
{
	if (mpv == nullptr) return;
	mpv_node args[3];
	args[0].format = MPV_FORMAT_STRING; args[0].u.string = "seek";
	args[1].format = MPV_FORMAT_DOUBLE; args[1].u.double_ = pos;
	args[2].format = MPV_FORMAT_STRING; args[2].u.string = "absolute";

	mpv_node command;
	mpv_node_list list{ 3, args };
	command.format = MPV_FORMAT_NODE_ARRAY;
	command.u.list = &list;

	mpv_command_node_async(mpv, 0, &command);
}

void MPVPlayer::setPlaySpeed(double speed)
{
	if (mpv == nullptr) return;
	mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &speed);
}

void MPVPlayer::setVolume(float volume)
{
	if (mpv == nullptr) return;
	double vol = volume * 100.0;
	mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
}

void MPVPlayer::setLoop(bool loop)
{
	if (mpv == nullptr) return;

	// "inf" = infinite loop, "no" = play once
	const char* value = loop ? "inf" : "no";
	mpv_set_property_string(mpv, "loop-file", value);
}


void MPVPlayer::setupAudio()
{
	if (fileInfo.numChannels <= 0)
	{
		return;
	}

	audioNodeID = AudioProcessorGraph::NodeID(AudioManager::getInstance()->getUniqueNodeGraphID());
	std::unique_ptr<MPVAudioProcessor> ap(new MPVAudioProcessor(this));
	audioProcessor = ap.get();
	AudioManager::getInstance()->graph.addNode(std::move(ap), audioNodeID);
	AudioManager::getInstance()->addAudioManagerListener(this);

	pipeThread->startThread();


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

void MPVPlayer::audioSetupChanged()
{
	setupAudio();
}



void MPVPlayer::onMPVUpdate()
{
	mpvListeners.call(&MPVListener::mpvFrameUpdate);
}

void MPVPlayer::onMPVWakeup()
{
}

void MPVPlayer::pullEvents()
{
	if (mpv == nullptr) return;

	while (true)
	{
		mpv_event* e = mpv_wait_event(mpv, 0);
		if (e->event_id == MPV_EVENT_NONE) break;

		switch (e->event_id)
		{
		case MPV_EVENT_FILE_LOADED:
		{
			//check actual number of tracks

			//retrieve width/height/duration/channels
			fileInfo.fileLoaded = false;
			pendingFileInfoMask = kMaskAll;
			mpv_get_property_async(mpv, kReqDuration, "duration", MPV_FORMAT_DOUBLE);
			mpv_get_property_async(mpv, kReqWidth, "width", MPV_FORMAT_INT64);
			mpv_get_property_async(mpv, kReqHeight, "height", MPV_FORMAT_INT64);
			mpv_get_property_async(mpv, kReqChannels, "audio-params/channel-count", MPV_FORMAT_INT64);
		}
		break;

		case MPV_EVENT_GET_PROPERTY_REPLY:
		{
			auto* prop = static_cast<mpv_event_property*>(e->data);
			switch (e->reply_userdata)
			{
			case kReqDuration:
				if (e->error == MPV_ERROR_SUCCESS && prop && prop->format == MPV_FORMAT_DOUBLE && prop->data)
					fileInfo.duration = *static_cast<double*>(prop->data);
				pendingFileInfoMask &= ~kMaskDuration;
				break;
			case kReqWidth:
				if (e->error == MPV_ERROR_SUCCESS && prop && prop->format == MPV_FORMAT_INT64 && prop->data)
					fileInfo.width = (int)*static_cast<int64_t*>(prop->data);
				pendingFileInfoMask &= ~kMaskWidth;
				break;
			case kReqHeight:
				if (e->error == MPV_ERROR_SUCCESS && prop && prop->format == MPV_FORMAT_INT64 && prop->data)
					fileInfo.height = (int)*static_cast<int64_t*>(prop->data);
				pendingFileInfoMask &= ~kMaskHeight;
				break;
			case kReqChannels:
				if (e->error == MPV_ERROR_SUCCESS && prop && prop->format == MPV_FORMAT_INT64 && prop->data)
					fileInfo.numChannels = (int)*static_cast<int64_t*>(prop->data);
				pendingFileInfoMask &= ~kMaskChannels;
				break;
			default:
				break;
			}

			if (pendingFileInfoMask == 0 && !fileInfo.fileLoaded)
			{
				setupAudio();
				mpvListeners.call(&MPVListener::mpvFileLoaded);
				fileInfo.fileLoaded = true;
			}
		}
		break;

		case MPV_EVENT_END_FILE:
			mpvListeners.call(&MPVListener::mpvFileEnd);
			break;

		case MPV_EVENT_PROPERTY_CHANGE:
		{
			mpv_event_property* prop = (mpv_event_property*)e->data;
			String pName = String(prop->name);
			if (pName == "time-pos" && prop->format == MPV_FORMAT_DOUBLE) {
				mpvListeners.call(&MPVListener::mpvTimeChanged, *(double*)prop->data);
			}
		}
		break;

		}
	}
}

int MPVPlayer::getMPVIntProperty(const char* name)
{
	if (mpv == nullptr) return 0;
	int64_t result = 0;
	mpv_get_property(mpv, name, MPV_FORMAT_INT64, &result);
	return (int)result;
}

double MPVPlayer::getMPVDoubleProperty(const char* name)
{
	if (mpv == nullptr) return 0;
	double result = 0;
	mpv_get_property(mpv, name, MPV_FORMAT_DOUBLE, &result);
	return result;
}

String MPVPlayer::getMPVStringProperty(const char* name)
{
	if (mpv == nullptr) return "";
	char* result = nullptr;
	mpv_get_property(mpv, name, MPV_FORMAT_STRING, &result);
	String strResult = String(result);
	mpv_free(result);
	return strResult;
}

int MPVPlayer::getVideoWidth()
{
	return fileInfo.width;
}
int MPVPlayer::getVideoHeight()
{
	return fileInfo.height;
}

double MPVPlayer::getDuration()
{
	return fileInfo.duration;
}

int MPVPlayer::getNumChannels()
{
	return fileInfo.numChannels;
}


bool MPVPlayer::isPlaying()
{
	if (mpv == nullptr) return false;
	int paused = 1;
	mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
	return (paused == 0);
}




// ==============================================================================
//Audio Pipe Thread

// PIPE THREAD

MPVPlayer::AudioPipeThread::AudioPipeThread(MPVPlayer* owner, String path)
	: Thread("MPV Audio Pipe"), owner(owner), pipePath(path)
{
#if JUCE_WINDOWS
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
#else
#endif

	readBuffer.resize(4096); // 4k buffer for reading chunks
}

MPVPlayer::AudioPipeThread::~AudioPipeThread()
{
	shutdown();
}

void MPVPlayer::AudioPipeThread::shutdown()
{
	stopThread(1000);
#if JUCE_WINDOWS
	if (pipeHandle != INVALID_HANDLE_VALUE) {
		DisconnectNamedPipe(pipeHandle);
		CloseHandle(pipeHandle);
		pipeHandle = INVALID_HANDLE_VALUE;
	}
#else
    
#endif
}

void MPVPlayer::AudioPipeThread::run()
{
#if JUCE_WINDOWS
	if (pipeHandle == INVALID_HANDLE_VALUE) return;

	// Wait for MPV to connect
	connected = ConnectNamedPipe(pipeHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	//LOG("Connected");

	if (connected)
	{
		DWORD bytesAvail = 0;
		while (!threadShouldExit())
		{
			if (PeekNamedPipe(pipeHandle, NULL, 0, NULL, &bytesAvail, NULL))
			{
				//LOG("Bytes available: " << bytesAvail);
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
					//LOG("No bytes available");
					// Pipe is connected but no audio is flowing (Paused/Stopped).
					// Sleep briefly to let the thread loop check 'threadShouldExit()'
					wait(5);
				}
			}
			else
			{
				//LOG("Pipe Peek failed");
				// Peek failed (Pipe disconnected or broken). 
				// Sleep briefly to avoid 100% CPU usage loop.
				wait(5);
			}
		}
	}
#else
#endif
}



// MPV Timers

juce_ImplementSingleton(MPVTimers);

MPVTimers::MPVTimers()
{
	startTimerHz(60);
}

MPVTimers::~MPVTimers() {
	stopTimer();
	players.clear();
}

void MPVTimers::registerMPV(MPVPlayer* vm)
{
	players.addIfNotAlreadyThere(vm);
}

void MPVTimers::unregisterMPV(MPVPlayer* vm)
{
	players.removeAllInstancesOf(vm);
}

void MPVTimers::timerCallback()
{
	for (auto p : players)
	{
		p->pullEvents();
	}
}
