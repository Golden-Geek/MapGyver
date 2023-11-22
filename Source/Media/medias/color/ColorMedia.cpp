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
	color = addColorParameter("Color", "", Colour(255, 255, 255));
}

ColorMedia::~ColorMedia()
{
}

void ColorMedia::clearItem()
{
	BaseItem::clearItem();
}

void ColorMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == color) shouldRedraw = true;
}

void ColorMedia::renderGL()
{
	Colour c = color->getColor();

	Init2DViewport(100, 100);

	glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Draw2DRect(0, 0, 20, 20);
}

Point<int> ColorMedia::getMediaSize()
{
	return Point<int>(100, 100);
}


