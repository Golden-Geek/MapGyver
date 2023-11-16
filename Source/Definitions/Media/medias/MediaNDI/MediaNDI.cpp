/*
  ==============================================================================

	MediaNDI.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "MediaNDI.h"
//#include "NDIManager.h"

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

void MediaNDI::onContainerParameterChanged(Parameter* p)
{
}

