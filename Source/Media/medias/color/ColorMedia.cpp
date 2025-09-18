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
	color = mediaParams.addColorParameter("Color", "", Colour(10,100,200));
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

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Draw2DRect(0, 0, 20, 20);
}

Point<int> ColorMedia::getDefaultMediaSize()
{
	return Point<int>(1, 1);
}


