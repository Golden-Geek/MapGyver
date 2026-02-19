/*
  ==============================================================================

	MediaListItemUI.cpp
	Created: 19 Feb 2026 9:59:28am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaListItemUI.h"

MediaListItemUI::MediaListItemUI(MediaListItem* item) :
	ItemUI(item)
{
	triggerButton.reset(item->launch->createButtonUI());
	addAndMakeVisible(triggerButton.get());

	if (item->media != nullptr) item->media->addAsyncInspectableListener(this);
	if (item->shaderMedia != nullptr) item->shaderMedia->addAsyncInspectableListener(this);
}

MediaListItemUI::~MediaListItemUI()
{
	if (inspectable.wasObjectDeleted()) return;

	if (item->media != nullptr) item->media->removeAsyncInspectableListener(this);
	if (item->shaderMedia != nullptr) item->shaderMedia->removeAsyncInspectableListener(this);

}

void MediaListItemUI::paint(juce::Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;
	ItemUI::paint(g);

	float w = item->weight->floatValue();
	Colour c = w == 1.f ? BLUE_COLOR : item->isLoading() ? Colours::orange : Colours::grey;
	if (w > 0.f)
	{
		Rectangle<int> r = getLocalBounds().removeFromBottom(w * getHeight());
		g.setColour(c);
		if (w == 1.f)
		{
			g.drawRoundedRectangle(r.toFloat(), 2.f, 2.f);
		}
		else
		{
			g.fillRoundedRectangle(r.toFloat(), 2.f);
		}
	}

	if (item->shaderMedia != nullptr && item->shaderMedia->enabled->boolValue())
	{
		g.setColour(Colours::lightblue);
		g.fillRoundedRectangle(transitionArea.toFloat(), 2.f);
	}


	Image img = item->media != nullptr ? item->media->previewImage : Image();
	g.drawImage(img, previewArea.toFloat(), RectanglePlacement::centred);

	if (item->media->isSelected)
	{
		Colour c = HIGHLIGHT_COLOR;
		g.setColour(c);
		g.drawRoundedRectangle(previewArea.toFloat(), 2.f, 2.f);
	}

	if (item->shaderMedia->isSelected)
	{
		Colour c = HIGHLIGHT_COLOR;
		g.setColour(c);
		g.drawRoundedRectangle(transitionArea.toFloat(), 2.f, 2.f);
	}

}

void MediaListItemUI::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (inspectable.wasObjectDeleted()) return;
	triggerButton->setBounds(r.removeFromLeft(30).reduced(2));
}

void MediaListItemUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	if (inspectable.wasObjectDeleted()) return;

	Rectangle<int> cr(r);
	int transitionWidth = item->shaderMedia->enabled->boolValue() ? 20 : 0;
	transitionArea = cr.removeFromLeft(transitionWidth).reduced(2);
	previewArea = cr.reduced(2);
}

void MediaListItemUI::mouseDown(const juce::MouseEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;

	if (e.mods.isAltDown())
	{
		item->launch->trigger();
	}
	else
	{
		if (previewArea.contains(e.getPosition()))
		{
			if (item->media != nullptr)
				item->media->selectThis();
		}
		else if (transitionArea.contains(e.getPosition()))
		{
			item->shaderMedia->selectThis();
		}
		else
		{
			ItemUI::mouseDown(e);
		}
	}
}

void MediaListItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (inspectable.wasObjectDeleted()) return;

	ItemUI::controllableFeedbackUpdateInternal(c);

	if (c == item->shaderMedia->enabled)
	{
		// Handle shader media feedback
		resized();
		repaint();
	}
	else if (c == item->weight)
	{
		repaint();
	}
}

void MediaListItemUI::newMessage(const Inspectable::InspectableEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	if (e.type == Inspectable::InspectableEvent::SELECTION_CHANGED)
	{

		repaint();
	}
}
