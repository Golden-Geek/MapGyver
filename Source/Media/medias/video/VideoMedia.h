/*
  ==============================================================================

	VideoMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class VideoMediaAudioProcessor;

class AudioFIFO
{
public:
	AudioFIFO(int numChannels, int size) :
		channels(numChannels),
		bufferSize(size)
	{
		fifoBuffer.setSize(channels, bufferSize);
		fifoBuffer.clear();
	}

	void pushData(const void* data, int totalSamples);
	void pullData(AudioBuffer<float>& buffer, int numSamples);

	bool hasData() const
	{
		return readPos.load() != writePos.load();
	}

	int getFramesAvailable() const
	{
		const auto localWritePos = writePos.load(std::memory_order_acquire);
		const auto localReadPos = readPos.load(std::memory_order_relaxed);
		return (localWritePos - localReadPos + bufferSize) % bufferSize;
	}

private:
	int channels;
	int bufferSize;
	AudioBuffer<float> fifoBuffer;

	std::atomic<int> readPos{ 0 };
	std::atomic<int> writePos{ 0 };
};


class VideoMedia :
	public ImageMedia,
	public AudioManager::AudioManagerListener
	//public Thread
{
public:
	VideoMedia(var params = var());
	~VideoMedia();

	void clearItem() override;

	enum VideoSource { Source_File, Source_URL };
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;

	enum PlayerState { UNLOADED, IDLE, READY, PLAYING, PAUSED, STATES_MAX };
	const String playerStateNames[STATES_MAX] = { "Unloaded", "Idle", "Ready", "Playing", "Paused" };
	EnumParameter* state;
	FloatParameter* position;
	FloatParameter* length;

	ControllableContainer controlsCC;
	Trigger* playTrigger;
	Trigger* stopTrigger;
	Trigger* pauseTrigger;
	Trigger* restartTrigger;
	BoolParameter* playAtLoad;
	BoolParameter* loop;
	FloatParameter* playSpeed;

	ControllableContainer audioCC;
	FloatParameter* volume;

	VLC::Instance* vlcInstance;
	std::unique_ptr<VLC::MediaPlayer> vlcPlayer;
	std::unique_ptr<VLC::Media> vlcMedia;

	int imageWidth = 0;
	int imageHeight = 0;
	int imagePitches = 0;
	int imageLines = 0;

	double frameRate;
	double totalFrames;

	bool updatingPosFromVLC;
	bool manuallySeeking;
	uint32 timeAtLastSeek;

	//Audio
	AudioProcessorGraph::NodeID audioNodeID;
	VideoMediaAudioProcessor* audioProcessor;

	void onContainerParameterChanged(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void setupAudio();

	void audioSetupChanged() override;

	void load();
	void play();
	void stop();
	void pause();
	void restart();
	void seek(double time);

	virtual void handleEnter(double time, bool play = false) override;
	virtual void handleExit() override;
	virtual void handleSeek(double time) override;
	virtual void handleStop() override;
	virtual void handleStart() override;

	bool isPlaying();

	double getMediaLength() override;

	String getMediaContentName() const override;

	void afterLoadJSONDataInternal() override;

	//void tapTempo();

	DECLARE_TYPE("Video")
};

class VideoMediaAudioProcessor :
	public AudioProcessor
{
public:
	VideoMediaAudioProcessor(VideoMedia* videoMedia);
	~VideoMediaAudioProcessor() override;

	VideoMedia* videoMedia;
	std::unique_ptr<AudioFIFO> fifo;
	
	// NEW MEMBERS
	std::atomic<bool> isBuffering{ true };
	int bufferThreshold = 0;

	void onAudioPlay(const void* data, unsigned int count, int64_t pts);
	void onAudioFlush(int64_t pts);


	void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

	AudioProcessorEditor* createEditor() override { return nullptr; }
	bool hasEditor() const override { return false; }

	virtual void getStateInformation(MemoryBlock& destData) override {}
	virtual void setStateInformation(const void* data, int sizeInBytes) override {}

	const String getName() const override { return videoMedia->niceName +" Processor"; }
	bool acceptsMidi() const override { return false; }
	bool producesMidi() const override { return false; }
	double getTailLengthSeconds() const override { return 0.0; }
	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int index) override {}
	const String getProgramName(int index) override { return {}; }
	void changeProgramName(int index, const String& newName) override {}
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override {}
};