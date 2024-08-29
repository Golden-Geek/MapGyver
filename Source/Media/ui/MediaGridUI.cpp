/*
  ==============================================================================

	MediaGridUI.cpp
	Created: 28 Aug 2024 11:46:38am
	Author:  bkupe

  ==============================================================================
*/

#include "../MediaIncludes.h"

MediaGridUI::MediaGridUI(Media* item) :
	BaseItemUI(item, NONE)
{
	preview.setMedia(item);
	addAndMakeVisible(&preview);
	bgColor = Colours::transparentBlack;
	preview.setInterceptsMouseClicks(false, false);

}

MediaGridUI::~MediaGridUI()
{
}

void MediaGridUI::paintOverChildren(Graphics& g)
{
	BaseItemUI::paintOverChildren(g);

}

void MediaGridUI::resizedInternalContent(Rectangle<int>& r)
{
	preview.setBounds(r.reduced(2));
}

void MediaGridUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == item->isBeingUsed)
	{
		repaint();
	}
}
