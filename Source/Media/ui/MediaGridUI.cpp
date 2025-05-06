/*
  ==============================================================================

	MediaGridUI.cpp
	Created: 28 Aug 2024 11:46:38am
	Author:  bkupe

  ==============================================================================
*/

#include "../MediaIncludes.h"

juce_ImplementSingleton(MediaGridUIPreview);

MediaGridUI::MediaGridUI(Media* item) :
	BaseItemUI(item, NONE),
	useLivePreview(false),
	useLiveOnHover(false)
{
	updatePreview();

	item->addAsyncMediaListener(this);
}

MediaGridUI::~MediaGridUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		item->removeAsyncMediaListener(this);
	}
}

void MediaGridUI::mouseEnter(const MouseEvent& e)
{
	updatePreview();
}

void MediaGridUI::mouseExit(const MouseEvent& e)
{
	updatePreview();
}

void MediaGridUI::setUseLivePreview(bool value)
{
	if (useLivePreview == value) return;
	useLivePreview = value;
	updatePreview();
}

void MediaGridUI::updatePreview()
{
	bool shouldUseLivePreview = useLivePreview || (useLiveOnHover && isMouseOver());

	if (shouldUseLivePreview)
	{
		MediaGridUIPreview::getInstance()->setMedia(item);
		addAndMakeVisible(MediaGridUIPreview::getInstance());
	}
	else
	{
		if (MediaGridUIPreview::getInstanceWithoutCreating() != nullptr)
		{
			removeChildComponent(MediaGridUIPreview::getInstance());
			MediaGridUIPreview::getInstance()->setMedia(nullptr);
		}
	}

	resized();
	repaint();
}

void MediaGridUI::paint(Graphics& g)
{
	BaseItemUI::paint(g);

	if (inspectable.wasObjectDeleted()) return;

	bool usingLivePreview = isParentOf(MediaGridUIPreview::getInstanceWithoutCreating());
	if (!usingLivePreview && item->previewImage.isValid())
	{
		g.drawImage(item->previewImage, previewBounds.toFloat(), RectanglePlacement());
	}
}


void MediaGridUI::resizedInternalContent(Rectangle<int>& r)
{
	previewBounds = r.reduced(2);
	bool usingLivePreview = isParentOf(MediaGridUIPreview::getInstanceWithoutCreating());
	if (usingLivePreview) MediaGridUIPreview::getInstance()->setBounds(previewBounds);
}

void MediaGridUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == item->isBeingUsed)
	{
		repaint();
	}
}

void MediaGridUI::newMessage(const Media::MediaEvent& e)
{
	if (e.type == Media::MediaEvent::PREVIEW_CHANGED || e.type == Media::MediaEvent::EDITING_CHANGED)
	{
		repaint();
	}
}
