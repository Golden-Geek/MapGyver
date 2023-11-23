/*
  ==============================================================================

    USBCamMedia.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class USBCamMedia :
    public ImageMedia,
    public USBCamInputDevice::USBCamInputListener
{
public:
    USBCamMedia(var params = var());
    ~USBCamMedia();

    USBCamDeviceParameter* USBCamParam;
    USBCamInputDevice* USBCamDevice = nullptr;
    ColorParameter* color;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;

    void updateDevice();

    void usbCamImageReceived(const Image& image) override;

    DECLARE_TYPE("USBCam")
};