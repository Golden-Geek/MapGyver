/*
  ==============================================================================

	ColorMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

ColorMedia::ColorMedia(var params) :
	Media(getTypeString(), params)
{
	color = mediaParams.addColorParameter("Color", "", Colour(10, 100, 200));
}

ColorMedia::~ColorMedia()
{
}

void ColorMedia::clearItem()
{
	BaseItem::clearItem();
}


void ColorMedia::renderGLInternal()
{

	Init2DViewport(1, 1);

	Colour c = color->getColor();
	glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
	glClear(GL_COLOR_BUFFER_BIT);

}

Point<int> ColorMedia::getDefaultMediaSize()
{
	return Point<int>(1, 1);
}


