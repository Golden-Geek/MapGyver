/*
  ==============================================================================

	MediaNDI.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaNDI::MediaNDI(var params) :
	Media(getTypeString(), params)
{
	color = addColorParameter("Color", "", Colour(255,0,0));
	ndiParam = new NDIDeviceParameter("NDI Source");
	addParameter(ndiParam);
	//NDIManager::getInstance();
}

MediaNDI::~MediaNDI()
{
	
}

void MediaNDI::clearItem()
{
	BaseItem::clearItem();
}

void MediaNDI::onContainerParameterChangedInternal(Parameter* p)
{
    if (p == ndiParam) {
        updateDevice();
    }
}

void MediaNDI::updateDevice()
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

void MediaNDI::videoFrameReceived(NDIlib_video_frame_v2_t* frame)
{
    const uint8_t* frameData = frame->p_data;
    int width = frame->xres;
    int height = frame->yres;

    // Créer une image JUCE et copier les données
    imageLock.enter();

    if (image.getWidth() != width || image.getHeight() != height) {
        image = Image(Image::PixelFormat::RGB,width, height, false);
        bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::ReadWriteMode::writeOnly);
    }
    NdiVideoHelper::convertVideoFrame(frame, image);
    //std::memcpy(bitmapData->data, frameData, width * height * 2); // 
    imageLock.exit();
    updateVersion();
}



    