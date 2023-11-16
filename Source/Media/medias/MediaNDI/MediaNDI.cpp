/*
  ==============================================================================

	MediaNDI.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaNDI::MediaNDI(var params) :
	Media(params)
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

