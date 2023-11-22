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
	//frameBuffer.clear(c);

	glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, frameBuffer.getWidth(), frameBuffer.getHeight(), 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 1); glVertex2f(0, 0);
	glTexCoord2f(0, 0); glVertex2f(0, 20);
	glTexCoord2f(1, 0); glVertex2f(20, 20);
	glTexCoord2f(1, 1); glVertex2f(20, 0);
	glEnd();
}

Point<int> ColorMedia::getMediaSize()
{
	return Point<int>(100, 100);
}


