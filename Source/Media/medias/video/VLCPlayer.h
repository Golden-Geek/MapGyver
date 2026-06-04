/*
  ==============================================================================

	VLCPlayer.h
	VLC-based video playback engine

  ==============================================================================
*/

#pragma once

#ifdef VLC_ENABLE

#include <vlc/vlc.h>
#include "VideoPlayerEngine.h"

class VLCAudioProcessor;

class VLCPlayer : public VideoPlayerEngine
{
public:
	VLCPlayer(const String& filePath);
	~VLCPlayer();

	// VideoPlayerEngine implementation
	bool load(const juce::String& filePath) override;
	void unload() override;

	void play() override;
	void pause() override;
	void stop() override;
	bool isPlaying() const override;

	void setPosition(double pos) override;
	double getPosition() const override;
	double getDuration() const override;

	void setPlaySpeed(float speed) override;
	float getPlaySpeed() const override;
	void setVolume(float volume) override;
	float getVolume() const override;
	void setLoop(bool loop) override;
	bool getLoop() const override;

	int getVideoWidth() const override;
	int getVideoHeight() const override;
	int getNumChannels() const override;

	void setupGL() override;
	void renderGL(int width, int height) override;

	juce::AudioProcessor* getAudioProcessor() override;

	void pullEvents() override;

private:
	void setupVLC();
	void setupAudio();
	void handleVLCEvent(const libvlc_event_t* event);

	static void vlcEventCallback(const libvlc_event_t* event, void* userData);
	static void* vlcLockCallback(void* opaque, void** planes);
	static void vlcUnlockCallback(void* opaque, void* picture, void* const* planes);
	static void vlcDisplayCallback(void* opaque, void* picture);
	static void vlcAudioPlayCallback(void* data, const void* samples, unsigned count, int64_t pts);
	static void vlcAudioFlushCallback(void* data, int64_t pts);

public:
	juce::Image getVideoFrame() const;

	String filePath;

	libvlc_instance_t* vlcInstance = nullptr;
	libvlc_media_player_t* mediaPlayer = nullptr;
	libvlc_media_t* currentMedia = nullptr;

	std::unique_ptr<VLCAudioProcessor> audioProcessor;

	// Video frame buffer
	juce::CriticalSection videoLock;
	std::vector<uint8_t> videoBuffer;
	juce::Image videoFrame;
	int videoWidth = 0;
	int videoHeight = 0;
	bool frameReady = false;

	// Playback state
	std::atomic<bool> playing{ false };
	std::atomic<float> currentSpeed{ 1.0f };
	std::atomic<float> currentVolume{ 1.0f };
	std::atomic<bool> loopEnabled{ false };
	std::atomic<int> numAudioChannels{ 2 };
	std::atomic<double> duration{ 0.0 };

	// Wall-clock interpolation anchor (written from VLC event thread, read from timer thread)
	std::atomic<int64_t> lastVLCTimeMs{ 0 };
	std::atomic<int64_t> lastVLCWallMs{ 0 };

	juce::String currentFile;
	bool isLoaded = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VLCPlayer)
};

// Audio processor for VLC
class VLCAudioProcessor : public juce::AudioProcessor
{
public:
	VLCAudioProcessor();
	~VLCAudioProcessor();

	void pushAudioData(const float* samples, int numSamples, int numChannels);
	void flush();

	// AudioProcessor implementation
	const juce::String getName() const override { return "VLC Audio"; }
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
	double getTailLengthSeconds() const override { return 0.0; }
	bool acceptsMidi() const override { return false; }
	bool producesMidi() const override { return false; }
	juce::AudioProcessorEditor* createEditor() override { return nullptr; }
	bool hasEditor() const override { return false; }
	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int index) override {}
	const juce::String getProgramName(int index) override { return {}; }
	void changeProgramName(int index, const juce::String& newName) override {}
	void getStateInformation(juce::MemoryBlock& destData) override {}
	void setStateInformation(const void* data, int sizeInBytes) override {}

private:
	juce::AudioBuffer<float> fifoBuffer;
	juce::CriticalSection audioLock;
	int writePosition = 0;
	int readPosition = 0;
	int availableSamples = 0;
	static const int fifoSize = 48000 * 4; // 4 seconds buffer

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VLCAudioProcessor)
};

#endif // VLC_ENABLE
