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
		if (c == playTrigger) play();
		else if (c == stopTrigger) stop();
		else if (c == pauseTrigger) pause();
		else if (c == restartTrigger) restart();
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
	NLOG(niceName, "Setup audio");
	AudioManager::getInstance()->graph.disconnectNode(audioNodeID);

	int sampleRate = AudioManager::getInstance()->graph.getSampleRate();
	int bufferSize = AudioManager::getInstance()->graph.getBlockSize();

	if (vlcMedia != nullptr)
	{

		//retrieve audio channels
		std::vector<VLC::MediaTrack> tracks = vlcMedia->tracks(VLC::MediaTrack::Audio);
		if (tracks.size() > 0)
		{
			VLC::MediaTrack track = tracks[0];
			int numChannels = track.channels();

			vlcPlayer->setAudioFormat("FL32", sampleRate, AudioManager::getInstance()->getNumUserOutputs());

			audioProcessor->setPlayConfigDetails(0, numChannels, sampleRate, bufferSize);
			audioProcessor->prepareToPlay(sampleRate, bufferSize);

			int minChannels = jmin(numChannels, AudioManager::getInstance()->getNumUserOutputs());
			for (int i = 0; i < minChannels; ++i)
			{
				bool canConnect = AudioManager::getInstance()->graph.canConnect({ { audioNodeID, i }, { AUDIO_OUTPUTMIXER_GRAPH_ID, i } });
				if (canConnect)
				{
					AudioManager::getInstance()->graph.addConnection({ { audioNodeID, i }, { AUDIO_OUTPUTMIXER_GRAPH_ID, i } });
					NLOG(niceName, "Connected audio channel " << i);
				}
				else NLOGWARNING(niceName, "Could not connect audio channel " << i);

			}

		
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

			//check audio here
			setupAudio();


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

	//libvlc_event_manager_t* em2 = libvlc_media_player_event_manager(vlcPlayer->get());
	//libvlc_event_attach(em2, libvlc_MediaPlayerPlaying,
	//	[](const libvlc_event_t*, void* ud) {
	//		((VideoMedia*)ud)->setupAudio();
	//	}, this);

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


bool VideoMedia::isPlaying()
{
	return state->getValueDataAsEnum<PlayerState>() == PLAYING;
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
	if (!videoMedia->isPlaying()) return;

	if (fifo != nullptr)
	{
		int numFrames = count / getTotalNumOutputChannels();
		LOG("Received " << numFrames << " frames from VLC");
		fifo->pushData(data, count);

		// If we are in a buffering state, check if we've crossed the threshold
		if (isBuffering && fifo->getFramesAvailable() >= bufferThreshold)
		{
			isBuffering = false; // We have enough data, tell the consumer it's ok to start pulling.
		}
	}

}

void VideoMediaAudioProcessor::onAudioFlush(int64_t pts)
{
	//LOG("Audio flush at pts: " << pts);
}

void VideoMediaAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{

	if (!videoMedia->isPlaying()) return;
	if (fifo == nullptr || buffer.getNumChannels() == 0)
	{
		buffer.clear();
		return;
	}

	// If we are buffering, output silence and wait for the producer to catch up.
	if (isBuffering)
	{
		LOG("Buffering... Frames available: " << fifo->getFramesAvailable());
		buffer.clear();
		return;
	}


	fifo->pullData(buffer, buffer.getNumSamples());
	float rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
	LOG("Pulling " << buffer.getNumSamples() << " samples from FIFO, remaining " << fifo->getFramesAvailable() << ", RMS : " << rms);

	// This is a new check inside pullData now, but as a safeguard,
	// if the buffer is ever empty after a pull, it means we've underrun.
	// Re-engage buffering to prevent crackles.
	if (rms < 1e-5 && fifo->getFramesAvailable() < buffer.getNumSamples())
	{
		LOG("Audio underrun detected, re-entering buffering state");
		isBuffering = true;
	}

}

void VideoMediaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	fifo.reset(new AudioFIFO(getTotalNumOutputChannels(), sampleRate * 10)); // 10 seconds buffer

	int numChannels = getTotalNumOutputChannels();
	if (numChannels > 0)
	{
		fifo.reset(new AudioFIFO(numChannels, (int)(sampleRate * 10))); // 10-second buffer is plenty

		// Set our low water mark. Let's wait for at least 4 blocks of audio.
		// This gives the producer a good head start.
		bufferThreshold = samplesPerBlock * 4;

		// Start in a buffering state
		isBuffering = true;
	}
}

// Push audio data from VLC into the FIFO buffer (Producer Thread)
void AudioFIFO::pushData(const void* data, int totalSamples)
{
	const float* inputData = static_cast<const float*>(data);
	const int numFrames = totalSamples / channels;

	// Load the current read position to check for available space.
	const auto localReadPos = readPos.load(std::memory_order_acquire);
	const auto localWritePos = writePos.load(std::memory_order_relaxed);

	// Check available space. -1 to leave one slot empty to distinguish full/empty.
	int availableSpace = (localReadPos - localWritePos - 1 + bufferSize) % bufferSize;
	if (numFrames > availableSpace)
	{
		// Buffer overflow, data will be lost.
		// In a real-world scenario, you might want to log this.
		return;
	}

	// De-interleave data, handling potential wrap-around in the circular buffer.
	for (int ch = 0; ch < channels; ++ch)
	{
		if (localWritePos + numFrames > bufferSize)
		{
			// Data will wrap around the end of the buffer.
			int framesToEnd = bufferSize - localWritePos;
			int framesFromStart = numFrames - framesToEnd;

			// First part: to the end of the buffer
			for (int i = 0; i < framesToEnd; ++i)
				fifoBuffer.getWritePointer(ch)[localWritePos + i] = inputData[i * channels + ch];

			// Second part: from the start of the buffer
			for (int i = 0; i < framesFromStart; ++i)
				fifoBuffer.getWritePointer(ch)[i] = inputData[(i + framesToEnd) * channels + ch];
		}
		else
		{
			// Data fits in a contiguous block.
			for (int i = 0; i < numFrames; ++i)
				fifoBuffer.getWritePointer(ch)[localWritePos + i] = inputData[i * channels + ch];
		}
	}

	// Atomically update the write position for the consumer thread to see.
	writePos.store((localWritePos + numFrames) % bufferSize, std::memory_order_release);

	LOG("Pushed " << numFrames << " frames to FIFO");
}

// Pull data from the FIFO buffer for processing by JUCE (Consumer Thread)
void AudioFIFO::pullData(AudioBuffer<float>& buffer, int numSamples)
{
	const int framesRequested = numSamples;

	const auto localWritePos = writePos.load(std::memory_order_acquire);
	const auto localReadPos = readPos.load(std::memory_order_relaxed);

	const int dataAvailable = (localWritePos - localReadPos + bufferSize) % bufferSize;
	const int framesToPull = std::min(dataAvailable, framesRequested);

	if (framesToPull == 0)
	{
		buffer.clear(); // Ensure buffer is silent if we have no data
		return;
	}

	// Copy data from our planar FIFO to the destination planar buffer.
	for (int ch = 0; ch < channels; ++ch)
	{
		if (ch >= buffer.getNumChannels()) break; // Safety check

		if (localReadPos + framesToPull > bufferSize)
		{
			// Data wraps around.
			int framesToEnd = bufferSize - localReadPos;
			int framesFromStart = framesToPull - framesToEnd;
			buffer.copyFrom(ch, 0, fifoBuffer, ch, localReadPos, framesToEnd);
			buffer.copyFrom(ch, framesToEnd, fifoBuffer, ch, 0, framesFromStart);
		}
		else
		{
			// Single contiguous block.
			buffer.copyFrom(ch, 0, fifoBuffer, ch, localReadPos, framesToPull);
		}
	}

	// Atomically update the read position.
	readPos.store((localReadPos + framesToPull) % bufferSize, std::memory_order_release);

	// If we couldn't provide all requested frames, clear the rest of the buffer.
	if (framesToPull < framesRequested)
	{
		buffer.clear(framesToPull, framesRequested - framesToPull);
	}

	LOG("Pulled " << framesToPull << " frames from FIFO");
}