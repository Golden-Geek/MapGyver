/*
  ==============================================================================

    NDIMedia.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once


class NDIMedia :
    public ImageMedia,
    public NDIInputDevice::NDIInputListener
{
public:
    NDIMedia(var params = var());
    ~NDIMedia();

    NDIDeviceParameter* ndiParam;
    NDIInputDevice* ndiDevice = nullptr;
    ColorParameter* color;

    void onContainerParameterChangedInternal(Parameter* p) override;

    void updateDevice();
    void videoFrameReceived(NDIlib_video_frame_v2_t* frame) override;


    DECLARE_TYPE("NDI")
};