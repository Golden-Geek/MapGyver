/*
  ==============================================================================

    MediaNDI.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once


class MediaNDI :
    public Media,
    public NDIInputDevice::NDIInputListener
{
public:
    MediaNDI(var params = var());
    ~MediaNDI();

    bool frameUpdated;

    NDIDeviceParameter* ndiParam;
    NDIInputDevice* ndiDevice = nullptr;
    ColorParameter* color;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    
    DECLARE_TYPE("NDI")

    void updateDevice();

    void videoFrameReceived(NDIlib_video_frame_v2_t* frame) override;

    void renderOpenGL();

    //virtual MediaUI* createUI() {return new MediaNDI(); };
};