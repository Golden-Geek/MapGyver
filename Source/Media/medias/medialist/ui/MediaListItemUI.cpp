/*
  ==============================================================================

	MediaListItemUI.cpp
	Created: 19 Feb 2026 9:59:28am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListItemUI::MediaListItemUI(MediaListItem* item) :
	ItemUI(item, HORIZONTAL)
{
	triggerButton.reset(item->launch->createButtonUI());
	addAndMakeVisible(triggerButton.get());

	item->addAsyncMediaListItemListener(this);
	//if (item->media != nullptr) item->media->addAsyncInspectableListener(this);
	//if (item->shaderMedia != nullptr) item->shaderMedia->addAsyncInspectableListener(this);
}

MediaListItemUI::~MediaListItemUI()
{
	if (inspectable.wasObjectDeleted()) return;
	item->removeAsyncContainerListener(this);

	//if (item->media != nullptr) item->media->removeAsyncInspectableListener(this);
	//if (item->shaderMedia != nullptr) item->shaderMedia->removeAsyncInspectableListener(this);

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

	for (int i = 0; i < item->subItems.size(); i++)
	{
		MediaListSubItem* s = item->subItems[i];
		if (s->media != nullptr && s->media->isSelected)
		{
			g.setColour(Colours::lightgreen);
			g.drawRoundedRectangle(subItemMediaAreas[i].toFloat(), 2.f, 2.f);
		}
		if (s->shaderMedia != nullptr && s->shaderMedia->enabled->boolValue())
		{
			g.setColour(Colours::lightblue);
			g.fillRoundedRectangle(subItemTransitionAreas[i].toFloat(), 2.f);
		}


		Image img = s->media != nullptr ? s->media->previewImage : Image();
		g.drawImage(img, subItemMediaAreas[i].toFloat(), RectanglePlacement::centred);

		if (s->media != nullptr && s->media->isSelected)
		{
			Colour c = HIGHLIGHT_COLOR;
			g.setColour(c);
			g.drawRoundedRectangle(subItemMediaAreas[i].toFloat(), 2.f, 2.f);
		}

		if (s->shaderMedia != nullptr && s->shaderMedia->isSelected)
		{
			Colour c = HIGHLIGHT_COLOR;
			g.setColour(c);
			g.drawRoundedRectangle(subItemTransitionAreas[i].toFloat(), 2.f, 2.f);
		}
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
	bool hasShader = false;
	for (auto& s : item->subItems)
	{
		if (s->shaderMedia->enabled->boolValue())
		{
			hasShader = true;
			break;
		}
	}
	int transitionWidth = hasShader ? 20 : 0;
	transitionArea = cr.removeFromLeft(transitionWidth).reduced(2);
	previewArea = cr.reduced(2);

	subItemMediaAreas.clear();
	subItemTransitionAreas.clear();

	for (int i = 0; i < item->subItems.size(); i++)
	{
		Rectangle<int> subItemArea = previewArea.getProportion(Rectangle<float>(0.f, i / (float)item->subItems.size(), 1.f, 1.f / item->subItems.size()));
		Rectangle<int> subTransitionArea = transitionArea.getProportion(Rectangle<float>(0.f, i / (float)item->subItems.size(), 1.f, 1.f / item->subItems.size()));

		subItemMediaAreas.add(subItemArea.reduced(2));
		subItemTransitionAreas.add(subTransitionArea.reduced(2));

	}
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
		for (int i = 0; i < item->subItems.size(); i++)
		{
			if (subItemMediaAreas[i].contains(e.getPosition()))
			{
				if (item->subItems[i]->media != nullptr)
					item->subItems[i]->media->selectThis();
				return;
			}
			else if (subItemTransitionAreas[i].contains(e.getPosition()))
			{
				if (item->subItems[i]->shaderMedia != nullptr && item->subItems[i]->shaderMedia->enabled->boolValue())
					item->subItems[i]->shaderMedia->selectThis();
				return;
			}
		}
		ItemUI::mouseDown(e);
	}
}

void MediaListItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (inspectable.wasObjectDeleted()) return;

	ItemUI::controllableFeedbackUpdateInternal(c);

	for (auto& s : item->subItems)
	{
		if (c == s->shaderMedia->enabled)
		{
			resized();
			repaint();
			return;
		}
	}

	if (c == item->weight)
	{
		repaint();
	}
}

void MediaListItemUI::newMessage(const MediaListItem::MediaListItemEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	if (e.type == MediaListItem::MediaListItemEvent::SELECTION_CHANGED)
	{
		repaint();
	}
}
