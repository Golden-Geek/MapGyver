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

void MediaComposition::run()
{
	double start = Time::getMillisecondCounterHiRes();
	double period = 1000.0f / fps->floatValue();
	// process
	bool needRepaint = imageNeedRepaint;
	for (int i = 0; i < layers.items.size() && !needRepaint; i++) {
		CompositionLayer* l = layers.items[i];
		Media* m = dynamic_cast<Media*>(l->media->targetContainer.get());
		if (l->enabled->boolValue() && m->enabled->boolValue()) {
			if (texturesVersions.contains(m) && texturesVersions.getReference(m) != (int)m->imageVersion) {
				needRepaint = true;
			}
		}
	}
	if (needRepaint) {
		repaintImage();
	}
}

void MediaComposition::repaintImage()
{
	if (workGraphics == nullptr) {return;}
	workImage.clear(workImage.getBounds());
	for (int i = 0; i < layers.items.size(); i++) {
		CompositionLayer* l = layers.items[i];
		Media* m = dynamic_cast<Media*>(l->media->targetContainer.get());
		if (m != nullptr && l->enabled->boolValue() && m->enabled->boolValue()) {
			AffineTransform t;
			t = t.scaled(l->size->x / m->image.getWidth(), l->size->y / m->image.getHeight());
			t = t.rotated(degreesToRadians(l->rotation->floatValue()));
			t = t.translated(l->position->x,l->position->y);
			workGraphics->setOpacity(l->alpha->floatValue());
			workGraphics->drawImageTransformed(m->image.createCopy(), t, false);
			texturesVersions.set(m, m->imageVersion);
		}
	}

	myGraphics->drawImageAt(workImage, 0,0);
	updateVersion();
}

void MediaComposition::updateImagesSize()
{
	workImage = Image(juce::Image::PixelFormat::RGB, resolution->x, resolution->y, true);
	workImage.clear(Rectangle<int>(0, 0, resolution->x, resolution->y), Colour(255.0f, 0.0f, 0.0f));
	workGraphics = std::make_shared<Graphics>(workImage);

	image = Image(juce::Image::PixelFormat::RGB, resolution->x, resolution->y, true);
	image.clear(Rectangle<int>(0, 0, resolution->x, resolution->y), Colour(255.0f, 0.0f, 0.0f));
	myGraphics = std::make_shared<Graphics>(image);
	
	repaintImage();
}

void MediaComposition::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	imageNeedRepaint = true;
}

