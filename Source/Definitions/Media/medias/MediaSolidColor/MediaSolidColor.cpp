/*
  ==============================================================================

	MediaSolidColor.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "MediaSolidColor.h"

MediaSolidColor::MediaSolidColor(var params) :
	Media(params)
{
	color = addColorParameter("Color", "", Colour(255,0,0));
}

MediaSolidColor::~MediaSolidColor()
{
}

void MediaSolidColor::clearItem()
{
	BaseItem::clearItem();
}

void MediaSolidColor::onContainerParameterChanged(Parameter* p)
{
	Media::onContainerParameterChanged(p);
}

