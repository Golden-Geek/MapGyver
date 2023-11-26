/*
  ==============================================================================

	WebcamMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

WebcamMedia::WebcamMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	WebcamParam = new WebcamDeviceParameter("Webcam Source");
	addParameter(WebcamParam);
}

WebcamMedia::~WebcamMedia()
{
	if (WebcamDevice != nullptr) WebcamDevice->removeWebcamInputListener(this);
}

void WebcamMedia::clearItem()
{
	BaseItem::clearItem();
}

void WebcamMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == WebcamParam) {
		updateDevice();
	}
}

void WebcamMedia::updateDevice()
{
	if (isClearing) return;
	if (WebcamParam->inputDevice != WebcamDevice)
	{
		if (WebcamDevice != nullptr) WebcamDevice->removeWebcamInputListener(this);
		WebcamDevice = WebcamParam->inputDevice;

		if (WebcamDevice != nullptr)
		{
			WebcamDevice->addWebcamInputListener(this);
			NLOG(niceName, "Now listening to Webcam Device : " << WebcamDevice->name);
		}
	}

}

void WebcamMedia::WebcamImageReceived(const Image& camImage) {
	initImage(camImage);
	shouldRedraw = true;

}