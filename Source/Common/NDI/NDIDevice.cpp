/*
  ==============================================================================

    NDIDevice.cpp
    Created: 20 Dec 2016 1:17:56pm
    Author:  Ben

  ==============================================================================
*/
#include "Common/CommonIncludes.h"

NDIDevice::NDIDevice(NDIlib_source_t& info, Type t) :
    name(info.p_ndi_name != nullptr ? info.p_ndi_name : ""),
    type(t)
{
}

NDIInputDevice::NDIInputDevice(NDIlib_source_t& info) :
    NDIDevice(info, NDI_IN),
    Thread("NDI Input"),
    sourceName(info.p_ndi_name != nullptr ? info.p_ndi_name : ""),
    sourceAddress(info.p_url_address != nullptr ? info.p_url_address : "")
{
    sourceInfo.p_ndi_name = sourceName.toRawUTF8();
    sourceInfo.p_url_address = sourceAddress.isNotEmpty() ? sourceAddress.toRawUTF8() : nullptr;
    startThread();
}

NDIInputDevice::~NDIInputDevice()
{
    stopThread(3000);
    const ScopedLock lock(receiverLock);
    shouldProcess = false;
    if (pNDI_recv != nullptr)
    {
        NDIlib_recv_destroy(pNDI_recv);
        pNDI_recv = nullptr;
    }
}

void NDIInputDevice::addNDIInputListener(NDIInputListener* newListener)
{
    const ScopedLock lock(receiverLock);
    inputListeners.add(newListener);
    if (inputListeners.size() == 1)
    {
        NDIlib_recv_create_v3_t recv_create_desc;
        recv_create_desc.color_format = NDIlib_recv_color_format_BGRX_BGRA;

        pNDI_recv = NDIlib_recv_create_v3(&recv_create_desc);
        if (pNDI_recv != nullptr)
        {
            NDIlib_recv_connect(pNDI_recv, &sourceInfo);
            shouldProcess = true;
            notify();
        }
        else
        {
            LOGERROR("Error creating NDI receiver");
        }
    }
}

void NDIInputDevice::removeNDIInputListener(NDIInputListener* listener)
{
    const ScopedLock lock(receiverLock);
    inputListeners.remove(listener);
    if (inputListeners.size() == 0)
    {
        shouldProcess = false;
        if (pNDI_recv != nullptr)
        {
            NDIlib_recv_destroy(pNDI_recv);
            pNDI_recv = nullptr;
        }
    }
}

void NDIInputDevice::run()
{
    while (!threadShouldExit())
    {
        bool active = false;
        {
            const ScopedLock lock(receiverLock);
            if (shouldProcess && pNDI_recv != nullptr)
            {
                active = true;
                NDIlib_video_frame_v2_t video_frame;
                NDIlib_audio_frame_v3_t audio_frame;

                switch (NDIlib_recv_capture_v3(pNDI_recv, &video_frame, &audio_frame, nullptr, 100))
                {
                case NDIlib_frame_type_video:
                    inputListeners.call(&NDIInputListener::videoFrameReceived, &video_frame);
                    NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
                    break;

                case NDIlib_frame_type_audio:
                    inputListeners.call(&NDIInputListener::audioFrameReceived, &audio_frame);
                    NDIlib_recv_free_audio_v3(pNDI_recv, &audio_frame);
                    break;

                default:
                    break;
                }
            }
        }
        if (!active)
            wait(30);
    }
}
