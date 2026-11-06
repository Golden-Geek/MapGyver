/*
  ==============================================================================

    NDIDevice.h
    Created: 20 Dec 2016 1:17:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class NDIDevice
{
public:
    enum Type { NDI_IN, NDI_OUT };
    NDIDevice(NDIlib_source_t& deviceName, Type t);
    virtual ~NDIDevice() {}

    String name;
    Type type;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NDIDevice)
};

class NDIInputDevice :
    public NDIDevice,
    public Thread
{
public:
    NDIInputDevice(NDIlib_source_t& info);
    ~NDIInputDevice();

    NDIlib_recv_instance_t pNDI_recv = nullptr;

    // Own the strings: the NDI finder only guarantees its source pointers until
    // the next call to NDIlib_find_get_current_sources().
    String sourceName;
    String sourceAddress;
    NDIlib_source_t sourceInfo;

    CriticalSection receiverLock;
    std::atomic<bool> shouldProcess{ false };
    void run() override;

    class NDIInputListener
    {
    public:
        virtual ~NDIInputListener() {}
        virtual void videoFrameReceived(NDIlib_video_frame_v2_t* frame) {}
        virtual void audioFrameReceived(const NDIlib_audio_frame_v3_t* frame) {}
    };

    ListenerList<NDIInputListener> inputListeners;
    void addNDIInputListener(NDIInputListener* newListener);
    void removeNDIInputListener(NDIInputListener* listener);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NDIInputDevice)
};
