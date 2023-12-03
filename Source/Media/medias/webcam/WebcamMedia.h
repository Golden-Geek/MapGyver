/*
  ==============================================================================

    WebcamMedia.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class WebcamMedia :
    public ImageMedia,
    public WebcamInputDevice::WebcamInputListener
{
public:
    WebcamMedia(var params = var());
    ~WebcamMedia();

    WebcamDeviceParameter* WebcamParam;
    WebcamInputDevice* WebcamDevice = nullptr;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;

    void updateDevice();

    void initImage(Image& newImage) override;
    void renderGLInternal() override;

    void WebcamImageReceived(const Image& image) override;

    DECLARE_TYPE("Webcam")
};