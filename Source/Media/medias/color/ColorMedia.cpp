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
	color = addColorParameter("Color", "", Colour(255,255,255));
	setColor(color->getColor());
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
	if (p == color) {
		setColor(color->getColor());
	}
}

void ColorMedia::setColor(Colour c)
{
	GlContextHolder::getInstance()->context.executeOnGLThread([this, c](OpenGLContext &context) {
		frameBuffer.initialise(GlContextHolder::getInstance()->context, 10, 10);
		frameBuffer.clear(c);
	}, true);
}

