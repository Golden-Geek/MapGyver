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

void WebcamMedia::initImage(Image& newImage)
{
	shouldRedraw = true;
	if (!newImage.isValid())
	{
		image = Image();
		return;
	}

	if (newImage.getWidth() != image.getWidth() || newImage.getHeight() != image.getHeight()) {
		image = newImage.createCopy();
		graphics = std::make_shared<Graphics>(image);
		bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::readWrite);
	}
	else {
		Image::BitmapData b(newImage, Image::BitmapData::readOnly);
		std::memcpy(bitmapData->data, b.data, b.size);
	}
}

void WebcamMedia::renderGLInternal()
{
	GenericScopedLock lock(imageLock);
	glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmapData->width, bitmapData->height, GL_BGR, GL_UNSIGNED_BYTE, bitmapData->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void WebcamMedia::WebcamImageReceived(const Image& camImage) {
	initImage(const_cast<Image&>(camImage));
	shouldRedraw = true;
	FPSTick();

}