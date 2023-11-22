/*
  ==============================================================================

	MediaComposition.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaComposition::MediaComposition(var params) :
	Media(getTypeString(), params)
{
	resolution = addPoint2DParameter("Resolution", "Size of your composition");
	var d; d.append(1920); d.append(1080);
	resolution->setDefaultValue(d);

	fps = addFloatParameter("FPS", "Frame per seconds", 60, 1, 320);
	addChildControllableContainer(&layers);

	updateImagesSize();

	//startThread(Thread::Priority::highest);
}

MediaComposition::~MediaComposition()
{
	//stopThread(1000);
}

void MediaComposition::clearItem()
{
	BaseItem::clearItem();
}

void MediaComposition::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == resolution) {
		updateImagesSize();
	}
}

void MediaComposition::renderOpenGL()
{
	
}

void MediaComposition::updateImagesSize()
{
	workImage = Image(juce::Image::PixelFormat::RGB, resolution->x, resolution->y, true);
	workImage.clear(Rectangle<int>(0, 0, resolution->x, resolution->y), Colour(255.0f, 0.0f, 0.0f));
	workGraphics = std::make_shared<Graphics>(workImage);

	image = Image(juce::Image::PixelFormat::RGB, resolution->x, resolution->y, true);
	image.clear(Rectangle<int>(0, 0, resolution->x, resolution->y), Colour(255.0f, 0.0f, 0.0f));
	myGraphics = std::make_shared<Graphics>(image);
	
}

void MediaComposition::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	imageNeedRepaint = true;
}

