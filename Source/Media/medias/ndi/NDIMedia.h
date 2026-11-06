/*
  ==============================================================================

    NDIMedia.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once

#include <array>
#include <atomic>

// Stereo SPSC FIFO: one NDI receive thread writes, the JUCE audio thread reads.
// The audio thread also performs linear sample-rate conversion if needed.
class NDIAudioProcessor : public AudioProcessor
{
public:
    NDIAudioProcessor();

    void pushAudio(const NDIlib_audio_frame_v3_t* frame);
    void setVolume(float newVolume) { volume.store(newVolume, std::memory_order_relaxed); }
    void setActive(bool shouldBeActive)
    {
        if (active.exchange(shouldBeActive, std::memory_order_acq_rel) != shouldBeActive)
            requestFlush();
    }
    void requestFlush() { flushRequested.store(true, std::memory_order_release); }

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override;
    void prepareToPlay(double sampleRate, int) override;
    void releaseResources() override {}

    const String getName() const override { return "NDI Audio Processor"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int, const String&) override {}
    void getStateInformation(MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    static constexpr int fifoCapacity = 96000;
    std::array<float, fifoCapacity> left{};
    std::array<float, fifoCapacity> right{};
    std::atomic<uint64_t> writePosition{ 0 };
    std::atomic<uint64_t> readPosition{ 0 };
    std::atomic<int> sourceRate{ 48000 };
    std::atomic<double> outputRate{ 48000.0 };
    std::atomic<float> volume{ 1.0f };
    std::atomic<bool> active{ false };
    std::atomic<bool> flushRequested{ false };
    double readFraction = 0.0;
    bool started = false;
};

class NDIMedia :
    public ImageMedia,
    public NDIInputDevice::NDIInputListener,
    public AudioManager::AudioManagerListener
{
public:
    NDIMedia(var params = var());
    ~NDIMedia();

    NDIDeviceParameter* ndiParam;
    NDIInputDevice* ndiDevice = nullptr;
    ColorParameter* color;
    FloatParameter* volume;

    AudioProcessorGraph::NodeID audioNodeID;
    NDIAudioProcessor* audioProcessor = nullptr;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void updateDevice();
    void videoFrameReceived(NDIlib_video_frame_v2_t* frame) override;
    void audioFrameReceived(const NDIlib_audio_frame_v3_t* frame) override;
    void audioSetupChanged() override;
    void setupAudio();

    DECLARE_TYPE("NDI")
};
