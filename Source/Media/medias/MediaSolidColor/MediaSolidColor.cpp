/*
  ==============================================================================

	MediaSolidColor.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaSolidColor::MediaSolidColor(var params) :
	Media(params)
{
	color = addColorParameter("Color", "", Colour(255,255,255));
	myImage = Image(juce::Image::PixelFormat::ARGB, 1,1,true);
	myImage.setPixelAt(0,0,color->getColor());
}

MediaSolidColor::~MediaSolidColor()
{
}

void MediaSolidColor::clearItem()
{
	BaseItem::clearItem();
}

void MediaSolidColor::onContainerParameterChangedInternal(Parameter* p)
{
	myImage.setPixelAt(0, 0, color->getColor());
	updateVersion();
}

