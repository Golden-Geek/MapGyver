/*
  ==============================================================================

	NDIMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

NDIMedia::NDIMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	color = addColorParameter("Color", "", Colour(255, 0, 0));
	ndiParam = new NDIDeviceParameter("NDI Source");
	addParameter(ndiParam);
}

NDIMedia::~NDIMedia()
{
	if (ndiDevice != nullptr) ndiDevice->removeNDIInputListener(this);

}

void NDIMedia::clearItem()
{
	BaseItem::clearItem();
}

void NDIMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == ndiParam) {
		updateDevice();
	}
}

void NDIMedia::updateDevice()
{
	if (isClearing) return;

	if (ndiParam->inputDevice != ndiDevice)
	{
		if (ndiDevice != nullptr) ndiDevice->removeNDIInputListener(this);
		ndiDevice = ndiParam->inputDevice;

		if (ndiDevice != nullptr)
		{
			ndiDevice->addNDIInputListener(this);
			NLOG(niceName, "Now listening to NDI Device : " << ndiDevice->name);
		}
	}

}

void NDIMedia::videoFrameReceived(NDIlib_video_frame_v2_t* frame)
{
	const uint8_t* frameData = frame->p_data;
	int width = frame->xres;
	int height = frame->yres;

	// Créer une image JUCE et copier les données
	GenericScopedLock lock(imageLock);

	if (image.getWidth() != width || image.getHeight() != height) {
		initImage(width, height);
	}

	std::memcpy(bitmapData->data, frameData, width * height * 4);

	shouldUpdateImage = true;
}


