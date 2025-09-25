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
	ImageMedia(getTypeString(), params),
	controlsCC("Controls"),
	audioCC("Audio"),
	updatingPosFromVLC(false),
	isSeeking(false),
	lastTapTempo(0)
	//Thread("VLC frame checker")
{

	//init vlc
	vlcInstance = dynamic_cast<MGEngine*>(Engine::mainEngine)->vlcInstance.get();
	vlcPlayer.reset(new VLC::MediaPlayer(*vlcInstance));
	//vlcPlayer->play();


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

	playTrigger = controlsCC.addTrigger("Play", "Play video");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop video");
	pauseTrigger = controlsCC.addTrigger("Pause", "Pause video");
	restartTrigger = controlsCC.addTrigger("Restart", "Restart video");
	playAtLoad = controlsCC.addBoolParameter("Play at load", "Play as soon as the video is loaded", false);
	loop = controlsCC.addBoolParameter("Loop", "Loop video", false);
	playSpeed = controlsCC.addFloatParameter("Speed", "Speed of video", 1, 0);


	volume = audioCC.addFloatParameter("Volume", "Volume of video", 1, 0, 1);

	beatPerCycle = audioCC.addIntParameter("Beat by cycles", "Number of tap tempo beats by cycle", 1, 1);
	tapTempoTrigger = audioCC.addTrigger("Tap tempo", "");


	addChildControllableContainer(&controlsCC);
	addChildControllableContainer(&audioCC);


	audioNodeID = AudioProcessorGraph::NodeID(AudioManager::getInstance()->getUniqueNodeGraphID());
	std::unique_ptr<VideoMediaAudioProcessor> ap = std::make_unique<VideoMediaAudioProcessor>(this);
	audioProcessor = ap.get();
	AudioManager::getInstance()->graph.addNode(std::move(ap), audioNodeID);

	AudioManager::getInstance()->addAudioManagerListener(this);

	customFPSTick = true;
}

VideoMedia::~VideoMedia()
{
	//stop();
	//stopThread(100);

	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->removeAudioManagerListener(this);
}

//void VideoMedia::clearItem()
//{
//	BaseItem::clearItem();
//}
//
void VideoMedia::onContainerParameterChanged(Parameter* p)
{
	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
		//stop();
	}

	else if (p == source || p == filePath || p == url)
	{
		load();
	}

	else if (p == position)
	{
		if (!updatingPosFromVLC) seek(position->doubleValue());
	}
}


void VideoMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	Media::onControllableFeedbackUpdateInternal(cc, c);

	if (cc == &controlsCC)
	{
		if (c == playTrigger) vlcPlayer->play();
		else if (c == stopTrigger) vlcPlayer->stopAsync();
		else if (c == pauseTrigger) vlcPlayer->pause();
		else if (c == restartTrigger) vlcPlayer->setPosition(0, true);
		else if (c == playSpeed) vlcPlayer->setRate(playSpeed->floatValue());
	}
	else if (cc == &audioCC)
	{
		if (c == volume) vlcPlayer->setVolume(volume->floatValue());
		else if (c == tapTempoTrigger) tapTempo();

	}
}

void VideoMedia::setupAudio()
{
	AudioManager::getInstance()->graph.disconnectNode(audioNodeID);

	vlcPlayer->setAudioFormat("FL32", AudioManager::getInstance()->graph.getSampleRate(), AudioManager::getInstance()->getNumUserOutputs());

	if (vlcMedia != nullptr)
	{
		//retrieve audio channels
		std::vector<VLC::MediaTrack> tracks = vlcMedia->tracks(VLC::MediaTrack::Audio);
		if (tracks.size() > 0)
		{
			VLC::MediaTrack track = tracks[0];
			int numChannels = track.channels();

			for (int i = 0; i < AudioManager::getInstance()->getNumUserOutputs(); ++i)
			{
				AudioManager::getInstance()->graph.addConnection({ { audioNodeID, i }, { AUDIO_OUTPUTMIXER_GRAPH_ID, i } });
			}

			audioProcessor->setPlayConfigDetails(0, numChannels, AudioManager::getInstance()->graph.getSampleRate(), AudioManager::getInstance()->graph.getBlockSize());
			audioProcessor->prepareToPlay(AudioManager::getInstance()->graph.getSampleRate(), AudioManager::getInstance()->graph.getBlockSize());

		}
	}
}

void VideoMedia::audioSetupChanged()
{
	setupAudio();
}


void VideoMedia::load()
{
	//stop();

	File f = filePath->getFile();
	if (!f.existsAsFile())
	{
		NLOGWARNING(niceName, "File not found : " << f.getFullPathName());
		state->setValueWithData(IDLE);
		return;
	}

	vlcMedia.reset(new VLC::Media(f.getFullPathName().toStdString(), VLC::Media::FromType::FromPath));
	vlcPlayer.reset(new VLC::MediaPlayer(*vlcInstance, *vlcMedia));



	vlcPlayer->setVideoFormatCallbacks(
		[this](char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines) -> unsigned {

			GenericScopedLock lock(imageLock);
			imageWidth = *width;
			imageHeight = *height;
			imagePitches = *pitches;
			imageLines = *lines;

			initImage(imageWidth, imageHeight);

			//vlcDataIsValid = true;
			memcpy(chroma, "BGRA", 4);
			(*pitches) = imageWidth * 4;
			(*lines) = imageHeight;


			length->setValue(vlcPlayer->length() / 1000.0);
			position->setValue(0);
			position->setRange(0, length->doubleValue());

			std::vector<VLC::MediaTrack> tracks = vlcPlayer->tracks(VLC::MediaTrack::Video, false);
			if (tracks.size() > 0)
			{
				VLC::MediaTrack track = tracks[0];
				frameRate = track.fpsNum() * 1.0 / track.fpsDen();
				totalFrames = floor(length->doubleValue() * frameRate);
			}

			imageLock.exit();

			return 1;
		},
		[this]() {

		});


	vlcPlayer->setVideoCallbacks(
		[this](void** data) -> void* {
			imageLock.enter();
			data[0] = bitmapData->getLinePointer(0);
			return nullptr;
		},
		[this](void* oldBuffer, void* const* pixels) {
			imageLock.exit();

			updatingPosFromVLC = true;

			if (!isSeeking)
			{
				position->setValue(vlcPlayer->time() / 1000.0);

				double currentFrame = round(position->doubleValue() * frameRate);
				if (currentFrame >= totalFrames - 2)
				{
					if (loop->boolValue()) vlcPlayer->setPosition(0, true);
				}
				updatingPosFromVLC = false;
			}
		},
		[this](void* data) {
			shouldRedraw = true;
			FPSTick();

		});

	//set dummy output to control audio from app
	//vlcPlayer->setAudioOutput("dummy");


	state->setValueWithData(IDLE);

	vlcPlayer->setAudioOutput("amem");
	vlcPlayer->setAudioCallbacks(
		[this](const void* data, unsigned int count, int64_t pts) {
			audioProcessor->onAudioPlay(data, count, pts);
		},
		nullptr, nullptr, nullptr, nullptr);

	//Parse before setup audio

	libvlc_event_manager_t* em2 = libvlc_media_player_event_manager(vlcPlayer->get());
	libvlc_event_attach(em2, libvlc_MediaPlayerPlaying,
		[](const libvlc_event_t*, void* ud) {
			((VideoMedia*)ud)->setupAudio();
		}, this);

	if (!isCurrentlyLoadingData && playAtLoad->boolValue()) vlcPlayer->play();
}

void VideoMedia::play() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();

	if (st == IDLE || st == PAUSED)
	{
		vlcPlayer->play();
		state->setValueWithData(PlayerState::PLAYING);
	}
}

void VideoMedia::stop() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED)
	{
		vlcPlayer->stopAsync();
		state->setValueWithData(PlayerState::IDLE);
	}
}

void VideoMedia::pause() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING)
	{
		vlcPlayer->pause();
		state->setValueWithData(PlayerState::PAUSED);
	}
}

void VideoMedia::restart() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED || st == IDLE)
	{
		vlcPlayer->setPosition(0, true);
		vlcPlayer->play();
		state->setValueWithData(PlayerState::PLAYING);
	}
}

void VideoMedia::seek(double time)
{
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED)
	{
		double targetTime = time;
		bool isEnd = false;
		if (loop->boolValue()) targetTime = fmod(targetTime, length->doubleValue());
		double maxTime = length->doubleValue() - 1.0 / frameRate;
		targetTime = jlimit(0., maxTime, targetTime);
		if (targetTime >= maxTime) isEnd = true;

		isSeeking = true;
		vlcPlayer->setTime(targetTime * 1000, true);
		isSeeking = false;

		if (!isEnd && st == PLAYING && vlcPlayer->state() != libvlc_Playing) vlcPlayer->play();
	}
}

void VideoMedia::tapTempo()
{
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTapTempo;
	lastTapTempo = now;
	if (delta < 3000) {
		delta = delta * (int)beatPerCycle->getValue();
		if (length->doubleValue() > 0)
		{
			double rate = length->doubleValue() / delta;
			playSpeed->setValue(rate);
		}
	}
}




void VideoMedia::handleEnter(double time, bool doPlay)
{
	Media::handleEnter(time, doPlay);

	if (vlcPlayer == nullptr) return;

	seek(time);

	bool isEnd = false;
	if (position->doubleValue() >= length->doubleValue() - 1.0 / frameRate) isEnd = true;

	if (doPlay && !isEnd) play();
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




double VideoMedia::getMediaLength()
{
	return length->doubleValue();
}

void VideoMedia::afterLoadJSONDataInternal()
{
	Media::afterLoadJSONDataInternal();
	if (playAtLoad->boolValue()) play();
}

VideoMediaAudioProcessor::VideoMediaAudioProcessor(VideoMedia* videoMedia) :
	videoMedia(videoMedia)
{
}

VideoMediaAudioProcessor::~VideoMediaAudioProcessor()
{
}

void VideoMediaAudioProcessor::onAudioPlay(const void* data, unsigned int count, int64_t pts)
{
	//LOG("On audio play at pts: " << pts << " with count: " << count);


	for (int i = 0; i < vlcBuffer.getNumChannels(); ++i)
	{
		int dataIndex = i * vlcBuffer.getNumSamples() * sizeof(float);
		vlcBuffer.copyFrom(i, 0, (float*)((float*)data + dataIndex), vlcBuffer.getNumSamples());
	}
}

void VideoMediaAudioProcessor::onAudioFlush(int64_t pts)
{
	//LOG("Audio flush at pts: " << pts);
}

void VideoMediaAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	buffer.clear();

	if (buffer.getNumChannels() == 0) return;

	for (int i = 0; i < buffer.getNumChannels(); ++i)
	{
		if (vlcBuffer.getNumSamples() < buffer.getNumSamples()) break;
		buffer.copyFrom(i, 0, vlcBuffer, i % vlcBuffer.getNumChannels(), 0, buffer.getNumSamples());
	}

	LOG(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
}

void VideoMediaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	vlcBuffer = AudioBuffer<float>(getTotalNumOutputChannels(), samplesPerBlock);
}
