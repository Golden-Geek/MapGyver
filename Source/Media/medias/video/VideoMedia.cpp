/*
  ==============================================================================

	VideoMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/MGEngine.h"
#include "Engine/MGSettings.h"



VideoMedia::VideoMedia(var params) :
	Media(getTypeString(), params),
	controlsCC("Controls"),
	audioCC("Audio"),
	updatingPosFromPlayer(false),
	manuallySeeking(false),
	timeAtLastSeek(0)
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

	customFPSTick = true;
}

VideoMedia::~VideoMedia()
{
	// Cleanup MPV

}

void VideoMedia::clearItem()
{
	// Start MPV's asynchronous video-chain shutdown while its renderer is still
	// registered. mpv_render_context_free() waits for that chain to go away, so
	// sending stop only after closeGLInternal can deadlock the GL worker.
	if (engine != nullptr)
		engine->unload();
	deferMPVCleanup = mpv != nullptr && mpv->isGLInit();

	// unregisterOpenGlRenderer synchronously invokes closeGLInternal on the GL
	// thread, so the MPV update callback has stopped before listeners are removed.
	Media::clearItem();

	if (engine != nullptr)
	{
		engine->removeListener(this);
		if (mpv != nullptr)
			mpv->removeMPVListener(this);

		if (deferMPVCleanup)
		{
			std::unique_ptr<MPVPlayer> deferredPlayer(static_cast<MPVPlayer*>(engine.release()));
			mpv = nullptr;
			MPVPlayer::destroyAfterShutdown(std::move(deferredPlayer));
		}
		else
		{
			mpv = nullptr;
			engine.reset();
		}
	}
}

void VideoMedia::setupEngine(const String& path)
{
	// Get the selected engine from MGSettings
	MGSettings::VideoEngine selectedEngine = MGSettings::VideoEngine::ENGINE_MPV;
	if (MGSettings::getInstanceWithoutCreating())
	{
		selectedEngine = MGSettings::getInstance()->videoPlaybackEngine->getValueDataAsEnum<MGSettings::VideoEngine>();
		DBG("VideoMedia: Selected engine from settings: " + String(selectedEngine == MGSettings::ENGINE_MPV ? "MPV" : "VLC"));
	}
	else
	{
		DBG("VideoMedia: MGSettings not available, defaulting to MPV");
	}

	// Create the appropriate engine based on settings
	switch (selectedEngine)
	{
	case MGSettings::ENGINE_MPV:
		DBG("VideoMedia: Creating MPV engine for: " + path);
		engine.reset(new MPVPlayer(path));
		break;

#ifdef VLC_ENABLE
	case MGSettings::ENGINE_VLC:
		DBG("VideoMedia: Creating VLC engine for: " + path);
		engine.reset(new VLCPlayer(path));
		break;
#endif

	default:
		DBG("VideoMedia: Unknown engine, falling back to MPV");
		engine.reset(new MPVPlayer(path)); // Fallback to MPV
		break;
	}

	engine->addListener(this);

	// Keep mpv raw pointer for backward compatibility with existing MPV-specific code
	mpv = dynamic_cast<MPVPlayer*>(engine.get());
	if (mpv)
	{
		mpv->addMPVListener(this);
		// MPV defers loading until GL is ready (setupGL -> loadFile)
	}
	else
	{
		// Non-MPV engines (e.g. VLC) need explicit load() since they don't use GL init
		engine->load(path);
	}

	shouldRedraw = true;
}

void VideoMedia::setupMPV(const String& path)
{
	setupEngine(path);
}

void VideoMedia::onContainerParameterChanged(Parameter* p)
{
	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
	}

	if (p == source || p == filePath || p == url)
	{
		load();
	}
	else if (p == position)
	{
		if (!updatingPosFromPlayer)
			seek(position->doubleValue());
	}
	else if (p == length)
	{
		position->setRange(0, length->doubleValue());
		mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_LENGTH_CHANGED, this));
	}
	if (p == loop)
	{
		if (engine != nullptr)
		{
			engine->setLoop(loop->boolValue());
		}
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
			if (engine != nullptr)
			{
				engine->setPlaySpeed(playSpeed->floatValue());
			}
		}
	}
	else if (cc == &audioCC)
	{
		if (c == volume)
		{
			if (engine != nullptr)
			{
				engine->setVolume(volume->floatValue());
			}
		}
	}
}


void VideoMedia::load()
{
	if (isCurrentlyLoadingData) return;

	String path;
	if (source->getValueDataAsEnum<VideoSource>() == Source_File)
	{
		File f = filePath->getFile();
		if (!f.existsAsFile())
		{
			if (!f.getFileNameWithoutExtension().isEmpty())
				NLOGWARNING(niceName, "File not found : " << f.getFullPathName());

			mpv = nullptr; // Clear raw pointer before engine is destroyed
			if (engine != nullptr)
			{
				engine->unload();
				engine.reset();
			}

			state->setValueWithData(UNLOADED);
			videoWidth = 0;
			videoHeight = 0;
			mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
			return;
		}
		path = f.getFullPathName();
	}
	else
	{
		path = url->stringValue();
	}

	checkIsYoutubeVideo();

	setupMPV(path);
}


void VideoMedia::initGLInternal()
{
}

void VideoMedia::renderOpenGL()
{
	if (isClearing) return;
	if (engine == nullptr) return;
	// For MPV, we still need the specific GL init check
	if (mpv != nullptr && !mpv->isGLInit()) mpv->setupGL();

	Media::renderOpenGL();
}

void VideoMedia::renderGLInternal()
{
	PlayerState ps = state->getValueDataAsEnum<PlayerState>();
	if (ps != PLAYING && ps != PAUSED) return;

	if (engine == nullptr)
	{
		glClearColor(.1f, .5f, .8f, .5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	// MPV-specific rendering (uses frameBuffer directly)
	if (mpv != nullptr)
	{
		mpv->renderGL(&frameBuffer);
	}
#ifdef VLC_ENABLE
	else if (VLCPlayer* vlc = dynamic_cast<VLCPlayer*>(engine.get()))
	{
		juce::Image frame = vlc->getVideoFrame();
		if (frame.isValid())
		{
			// Lazily create / recreate FBO when video dimensions change
			if (!vlcFBO.isValid() || vlcFBO.getWidth() != frame.getWidth() || vlcFBO.getHeight() != frame.getHeight())
			{
				if (vlcFBO.isValid()) vlcFBO.release();
				vlcFBO.initialise(GlContextHolder::getInstance()->context, frame.getWidth(), frame.getHeight());
			}

			// Upload the CPU frame into the FBO texture
			vlcFBO.makeCurrentAndClear();
			glBindTexture(GL_TEXTURE_2D, vlcFBO.getTextureID());
			juce::Image::BitmapData bitmapData(frame, juce::Image::BitmapData::readOnly);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.getWidth(), frame.getHeight(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmapData.data);
			vlcFBO.releaseAsRenderingTarget();
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Draw whatever is in the FBO (last valid frame stays if no new one yet)
		if (vlcFBO.isValid())
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glBindTexture(GL_TEXTURE_2D, vlcFBO.getTextureID());
			Draw2DTexRectFlipped(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
#endif
}

void VideoMedia::closeGLInternal()
{
	if (engine == nullptr) return;
	if (mpv != nullptr)
	{
		if (deferMPVCleanup)
			mpv->stopGLUpdates();
		else
			mpv->clearGL();
	}
#ifdef VLC_ENABLE
	if (vlcFBO.isValid()) vlcFBO.release();
#endif
}


// CONTROL
void VideoMedia::play() {
	if (engine == nullptr) return;
	engine->play();
	state->setValueWithData(PLAYING);
}

void VideoMedia::stop() {
	if (engine == nullptr) return;
	engine->stop();
	state->setValueWithData(PAUSED);
}

void VideoMedia::pause() {
	if (engine == nullptr) return;
	engine->pause();
	state->setValueWithData(PAUSED);
}

void VideoMedia::restart() {
	if (engine == nullptr) return;
	seek(0);
	play();
}

void VideoMedia::seek(double time)
{
	if (engine == nullptr) return;
	double target = jlimit(0.0, length->doubleValue(), time);
	engine->setPosition(target);
}

void VideoMedia::checkIsYoutubeVideo()
{
	bool isYTVideo = source->getValueDataAsEnum<VideoSource>() == Source_URL &&
		(url->stringValue().containsIgnoreCase("youtube") ||
			url->stringValue().containsIgnoreCase("youtu.be"));

	if (isYTVideo)
	{
#if JUCE_WINDOWS
		File f = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("yt-dlp.exe");
#elif JUCE_MAC
		File f = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("Resources").getChildFile("yt-dlp");
#endif

		if (f.existsAsFile()) return;

		AlertWindow::showAsync(MessageBoxOptions().withIconType(AlertWindow::QuestionIcon)
			.withTitle("YouTube-DLP Not Found")
			.withMessage("YouTube video playback requires youtube-dlp to be present in the application folder.\n Would you like to download it now?")
			.withButton("OK")
			.withButton("Cancel"),
			[&](int result) {

#if JUCE_WINDOWS
				URL downloadURL("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe");
				File targetFile = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("yt-dlp.exe");
#elif JUCE_MAC
				URL downloadURL("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_macos");
				File targetFile = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("Resources").getChildFile("yt-dlp");
#endif

				NLOG(niceName, "Downloading YouTube-DLP from " << downloadURL.toString(true));
				FileDownloader::getInstance()->addDownloadFile(downloadURL, targetFile, [&](bool success) {

					if (success)
					{
						NLOG(niceName, "YouTube-DLP downloaded and ready.");
						load();

					}
					else
					{
						NLOGERROR(niceName, "Failed to download YouTube-DLP.");
					}
					});

			});
	}
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

void VideoMedia::mpvFileLoaded()
{
	videoWidth = mpv->getVideoWidth();
	videoHeight = mpv->getVideoHeight();
	length->setValue(mpv->getDuration());

	mpv->setVolume(volume->floatValue());
	mpv->setPlaySpeed(playSpeed->doubleValue());
	mpv->setLoop(loop->boolValue());

	shouldGeneratePreviewImage = true;

	pause();
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
}

void VideoMedia::mpvTimeChanged(double time)
{
	updatingPosFromPlayer = true;
	position->setValue(time);
	updatingPosFromPlayer = false;
}

void VideoMedia::mpvFrameUpdate()
{
	shouldRedraw = true;
}

void VideoMedia::mpvFileEnd()
{
	state->setValueWithData(PAUSED);
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_FINISHED, this));
}

// VideoPlayerEngine::Listener implementations (delegate to legacy MPV callbacks)
void VideoMedia::playerFileLoaded()
{
	if (engine == nullptr) return;
	videoWidth = engine->getVideoWidth();
	videoHeight = engine->getVideoHeight();
	length->setValue(engine->getDuration());

	engine->setVolume(volume->floatValue());
	engine->setPlaySpeed(playSpeed->floatValue());
	engine->setLoop(loop->boolValue());

	shouldGeneratePreviewImage = true;

	pause();
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
}

void VideoMedia::playerTimeChanged(double time)
{
	updatingPosFromPlayer = true;
	position->setValue(time);
	updatingPosFromPlayer = false;
}

void VideoMedia::playerFrameUpdate()
{
	shouldRedraw = true;
}

void VideoMedia::playerFileEnd()
{
	state->setValueWithData(PAUSED);
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_FINISHED, this));
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

	load();
	if (playAtLoad->boolValue())
		play();
}
