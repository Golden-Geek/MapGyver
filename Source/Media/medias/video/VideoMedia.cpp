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
	Media::clearItem();
	if (mpv != nullptr) mpv->clear();
	stop();
}

void VideoMedia::setupMPV(const String& path)
{
	mpv.reset(new MPVPlayer(path));
	mpv->addMPVListener(this);
	shouldRedraw = true;
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
			mpv->setPlaySpeed(playSpeed->doubleValue());
		}
	}
	else if (cc == &audioCC)
	{
		if (c == volume)
		{
			if (mpv != nullptr)
			{
				mpv->setVolume(volume->floatValue());
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

			if (mpv != nullptr)
			{
				mpv->clear();
				mpv.reset();
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

	setupMPV(path);
}


void VideoMedia::initGLInternal()
{
}

void VideoMedia::renderOpenGL()
{
	if (isClearing) return;
	if (mpv == nullptr) return;
	if(!mpv->isGLInit()) mpv->setupGL();

	Media::renderOpenGL();
}

void VideoMedia::renderGLInternal()
{
	if (mpv == nullptr)
	{
		glClearColor(.1f, .5f, .8f, .5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	mpv->renderGL(&frameBuffer);
}

void VideoMedia::closeGLInternal()
{
	if (mpv == nullptr) return;
}


// CONTROL
void VideoMedia::play() {
	if (mpv == nullptr) return;
	mpv->play();
}

void VideoMedia::stop() {
	if (mpv == nullptr) return;
	mpv->stop();
	state->setValueWithData(PAUSED);
}

void VideoMedia::pause() {
	if (mpv == nullptr) return;
	mpv->pause();
	state->setValueWithData(PAUSED);
}

void VideoMedia::restart() {
	if (mpv == nullptr) return;
	seek(0);
	play();
}

void VideoMedia::seek(double time)
{
	if (mpv == nullptr) return;
	double target = jlimit(0.0, length->doubleValue(), time);
	mpv->setPosition(target);
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
	if (loop->boolValue())
	{
		seek(0);
		play();

	}
	else {
		state->setValueWithData(PAUSED);
	}
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