/*
  ==============================================================================

	ColorMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "ColorMedia.h"

ColorMedia::ColorMedia(var params) :
	Media(getTypeString(), params)
{
	color = mediaParams.addColorParameter("Color", "", Colour(10, 100, 200));

	addFrameBuffer("Checker", &checkFrameBuffer);
}

ColorMedia::~ColorMedia()
{
}

void ColorMedia::clearItem()
{
	BaseItem::clearItem();
}

void ColorMedia::initFrameBuffer()
{
	Media::initFrameBuffer();
	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (checkFrameBuffer.isValid()) checkFrameBuffer.release();
	checkFrameBuffer.initialise(GlContextHolder::getInstance()->context, 512, 512);
}

void ColorMedia::renderGLInternal()
{

	Init2DViewport(1, 1);

	Colour c = color->getColor();
	glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
	glClear(GL_COLOR_BUFFER_BIT);


	//draw checked on checkFrameBuffer

	checkFrameBuffer.makeCurrentRenderingTarget();
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	//draw checker pattern
	Init2DViewport(checkFrameBuffer.getWidth(), checkFrameBuffer.getHeight());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Colour c1 = c;
	Colour c2 = Colours::darkgrey;
	int checkSize = 20;
	for (int y = 0; y < checkFrameBuffer.getHeight(); y += checkSize)
	{
		for (int x = 0; x < checkFrameBuffer.getWidth(); x += checkSize)
		{
			bool isC1 = ((x / checkSize) % 2) == ((y / checkSize) % 2);
			Colour col = isC1 ? c1 : c2;
			glColor4f(col.getFloatRed(), col.getFloatGreen(), col.getFloatBlue(), col.getFloatAlpha());
			Draw2DRect(x, y, checkSize, checkSize);
		}
	}

	frameBuffer.makeCurrentRenderingTarget();

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Draw2DRect(0, 0, 20, 20);
}

Point<int> ColorMedia::getDefaultMediaSize()
{
	return Point<int>(1, 1);
}


