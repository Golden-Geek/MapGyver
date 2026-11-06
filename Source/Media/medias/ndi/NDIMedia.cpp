/*
  ==============================================================================

    NDIMedia.cpp
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

NDIAudioProcessor::NDIAudioProcessor() :
    AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true))
{
}

void NDIAudioProcessor::prepareToPlay(double sampleRate, int)
{
    outputRate.store(sampleRate > 0 ? sampleRate : 48000.0, std::memory_order_relaxed);
}

void NDIAudioProcessor::pushAudio(const NDIlib_audio_frame_v3_t* frame)
{
    if (!active.load(std::memory_order_acquire))
        return;

    if (frame == nullptr || frame->p_data == nullptr || frame->no_channels < 1 ||
        frame->no_samples <= 0 || frame->sample_rate <= 0 ||
        frame->FourCC != NDIlib_FourCC_audio_type_FLTP ||
        frame->channel_stride_in_bytes < frame->no_samples * (int)sizeof(float))
        return;

    const uint64_t write = writePosition.load(std::memory_order_relaxed);
    const uint64_t read = readPosition.load(std::memory_order_acquire);
    const uint64_t count = (uint64_t)frame->no_samples;

    // Never overwrite data being read by the audio callback.
    if (write - read + count >= fifoCapacity)
        return;

    const float* leftSource = reinterpret_cast<const float*>(frame->p_data);
    const float* rightSource = frame->no_channels > 1
        ? reinterpret_cast<const float*>(frame->p_data + frame->channel_stride_in_bytes)
        : leftSource;

    for (int i = 0; i < frame->no_samples; ++i)
    {
        const size_t index = (size_t)((write + (uint64_t)i) % fifoCapacity);
        left[index] = leftSource[i];
        right[index] = rightSource[i];
    }

    sourceRate.store(frame->sample_rate, std::memory_order_relaxed);
    writePosition.store(write + count, std::memory_order_release);
}

void NDIAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    buffer.clear();
    if (buffer.getNumChannels() == 0)
        return;

    uint64_t read = readPosition.load(std::memory_order_relaxed);
    const uint64_t write = writePosition.load(std::memory_order_acquire);

    if (flushRequested.exchange(false, std::memory_order_acq_rel) ||
        !active.load(std::memory_order_acquire))
    {
        readPosition.store(write, std::memory_order_release);
        readFraction = 0.0;
        started = false;
        return;
    }

    const int inRate = jmax(1, sourceRate.load(std::memory_order_relaxed));
    const double outRate = jmax(1.0, outputRate.load(std::memory_order_relaxed));
    const double step = (double)inRate / outRate;
    const float gain = volume.load(std::memory_order_relaxed);
    uint64_t available = write - read;

    // Bound latency: if a slow output falls behind, skip to the live edge.
    if (available > (uint64_t)(inRate / 5))
    {
        read = write - (uint64_t)jmax(2, inRate / 16);
        readFraction = 0.0;
        available = write - read;
    }

    if (!started)
    {
        if (available < (uint64_t)jmax(2, inRate / 50))
        {
            readPosition.store(read, std::memory_order_release);
            return;
        }
        started = true;
    }

    float* outputLeft = buffer.getWritePointer(0);
    float* outputRight = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        available = write - read;
        if (available < 2)
        {
            started = false;
            readFraction = 0.0;
            break;
        }

        const double nextFraction = readFraction + step;
        const uint64_t advance = (uint64_t)nextFraction;
        if (advance >= available)
        {
            started = false;
            readFraction = 0.0;
            break;
        }

        const size_t a = (size_t)(read % fifoCapacity);
        const size_t b = (a + 1) % fifoCapacity;
        const float l = (float)(left[a] + (left[b] - left[a]) * readFraction) * gain;
        const float r = (float)(right[a] + (right[b] - right[a]) * readFraction) * gain;
        outputLeft[i] = l;
        if (outputRight != nullptr)
            outputRight[i] = r;

        read += advance;
        readFraction = nextFraction - (double)advance;
    }

    readPosition.store(read, std::memory_order_release);
}

NDIMedia::NDIMedia(var params) :
    ImageMedia(getTypeString(), params)
{
    color = addColorParameter("Color", "", Colour(255, 0, 0));
    ndiParam = new NDIDeviceParameter("NDI Source");
    addParameter(ndiParam);
    volume = addFloatParameter("Volume", "NDI audio volume", 1, 0, 2);

    customFPSTick = true;

    if (auto* audioManager = AudioManager::getInstanceWithoutCreating())
    {
        audioManager->addAudioManagerListener(this);
        setupAudio();
    }
}

NDIMedia::~NDIMedia()
{
    if (ndiDevice != nullptr)
        ndiDevice->removeNDIInputListener(this);

    if (auto* audioManager = AudioManager::getInstanceWithoutCreating())
    {
        audioManager->removeAudioManagerListener(this);
        if (audioProcessor != nullptr)
        {
            audioManager->graph.removeNode(audioNodeID);
            audioProcessor = nullptr;
        }
    }
}

void NDIMedia::setupAudio()
{
    auto* audioManager = AudioManager::getInstanceWithoutCreating();
    if (audioManager == nullptr)
        return;

    if (audioProcessor == nullptr)
    {
        std::unique_ptr<NDIAudioProcessor> processor(new NDIAudioProcessor());
        audioProcessor = processor.get();
        audioNodeID = AudioProcessorGraph::NodeID(audioManager->getUniqueNodeGraphID());
        if (audioManager->graph.addNode(std::move(processor), audioNodeID) == nullptr)
        {
            audioProcessor = nullptr;
            return;
        }
    }
    else
    {
        audioManager->graph.disconnectNode(audioNodeID);
    }

    int sampleRate = (int)audioManager->graph.getSampleRate();
    int blockSize = audioManager->graph.getBlockSize();
    if (sampleRate <= 0) sampleRate = 48000;
    if (blockSize <= 0) blockSize = 512;

    audioProcessor->setPlayConfigDetails(0, 2, sampleRate, blockSize);
    audioProcessor->prepareToPlay(sampleRate, blockSize);
    audioProcessor->setVolume(volume->floatValue());
    audioProcessor->setActive(enabled->boolValue() && isBeingUsed->boolValue());

    const int outputs = jmin(2, audioManager->getNumUserOutputs());
    for (int channel = 0; channel < outputs; ++channel)
    {
        AudioProcessorGraph::Connection connection{ { audioNodeID, channel },
            { AUDIO_OUTPUTMIXER_GRAPH_ID, channel } };
        if (audioManager->graph.canConnect(connection))
            audioManager->graph.addConnection(connection);
    }
}

void NDIMedia::audioSetupChanged()
{
    setupAudio();
}

void NDIMedia::onContainerParameterChangedInternal(Parameter* p)
{
    if (p == ndiParam)
        updateDevice();
    else if (p == volume && audioProcessor != nullptr)
        audioProcessor->setVolume(volume->floatValue());
    else if ((p == isBeingUsed || p == enabled) && audioProcessor != nullptr)
        audioProcessor->setActive(enabled->boolValue() && isBeingUsed->boolValue());
}

void NDIMedia::updateDevice()
{
    if (isClearing) return;
    if (ndiParam->inputDevice != ndiDevice)
    {
        if (ndiDevice != nullptr)
            ndiDevice->removeNDIInputListener(this);

        ndiDevice = ndiParam->inputDevice;
        if (audioProcessor != nullptr)
            audioProcessor->requestFlush();

        if (ndiDevice != nullptr)
        {
            if (audioProcessor == nullptr)
                setupAudio();
            ndiDevice->addNDIInputListener(this);
            NLOG(niceName, "Now listening to NDI Device : " << ndiDevice->name);
        }
    }
}

void NDIMedia::audioFrameReceived(const NDIlib_audio_frame_v3_t* frame)
{
    if (audioProcessor != nullptr)
        audioProcessor->pushAudio(frame);
}

void NDIMedia::videoFrameReceived(NDIlib_video_frame_v2_t* frame)
{
    const uint8_t* frameData = frame->p_data;
    int width = frame->xres;
    int height = frame->yres;

    FPSTick();

    if (image.getWidth() != width || image.getHeight() != height)
        initImage(width, height);

    // OpenGL reads this bitmap on another thread.
    const ScopedLock lock(imageLock);
    if (bitmapData != nullptr && frameData != nullptr)
        std::memcpy(bitmapData->data, frameData, (size_t)width * (size_t)height * 4);

    shouldRedraw = true;
}
