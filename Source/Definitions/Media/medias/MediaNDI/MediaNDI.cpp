/*
  ==============================================================================

	MediaNDI.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "MediaNDI.h"

MediaNDI::MediaNDI(var params) :
	Media(params)
{
	color = addColorParameter("Color", "", Colour(255,0,0));
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

