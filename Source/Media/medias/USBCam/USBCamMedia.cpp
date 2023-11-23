/*
  ==============================================================================

	USBCamMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

USBCamMedia::USBCamMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	USBCamParam = new USBCamDeviceParameter("USBCam Source");
	addParameter(USBCamParam);
}

USBCamMedia::~USBCamMedia()
{
	if (USBCamDevice != nullptr) USBCamDevice->removeUSBCamInputListener(this);
}

void USBCamMedia::clearItem()
{
	BaseItem::clearItem();
}

void USBCamMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == USBCamParam) {
		updateDevice();
	}
}

void USBCamMedia::updateDevice()
{
	if (isClearing) return;
	if (USBCamParam->inputDevice != USBCamDevice)
	{
		if (USBCamDevice != nullptr) USBCamDevice->removeUSBCamInputListener(this);
		USBCamDevice = USBCamParam->inputDevice;

		if (USBCamDevice != nullptr)
		{
			USBCamDevice->addUSBCamInputListener(this);
			NLOG(niceName, "Now listening to USBCam Device : " << USBCamDevice->name);
		}
	}

}

void USBCamMedia::usbCamImageReceived(const Image& camImage) {
	initImage(camImage);
	shouldRedraw = true;

}