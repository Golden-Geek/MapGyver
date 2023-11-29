/*
  ==============================================================================

	WebcamMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "WebcamMedia.h"

WebcamMedia::WebcamMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	WebcamParam = new WebcamDeviceParameter("Webcam Source");
	addParameter(WebcamParam);

	currentFPS = addFloatParameter("current FPS", "", 0);
	currentFPS->isSavable = false;
	currentFPS->enabled = false;
}

WebcamMedia::~WebcamMedia()
{
	if (WebcamDevice != nullptr) WebcamDevice->removeWebcamInputListener(this);
}

void WebcamMedia::FPSTick()
{
	double currentTime = juce::Time::getMillisecondCounterHiRes();
	double elapsedMillis = currentTime - lastFPSTick;
	lastFPSTick = currentTime;

	// Calcul des FPS
	MessageManager::callAsync([this, elapsedMillis]()
	{
		currentFPS->setValue(1000.0 / elapsedMillis);
	});
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
	initImage(const_cast<Image&>(camImage));
	FPSTick();
	shouldRedraw = true;

}