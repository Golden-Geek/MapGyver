/*
  ==============================================================================

    GridClipUI.cpp
    Created: 11 Jun 2025 4:40:04pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

GridClipUI::GridClipUI(GridClip* item) :
	ItemMinimalUI(item)
{
}

GridClipUI::~GridClipUI()
{
}

void GridClipUI::paint(Graphics& g)
{
    g.setColour(BG_COLOR);
	g.fillRoundedRectangle(getMainBounds().toFloat(), 4.0f);
	g.setColour(BG_COLOR.darker(0.7f));
	g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 4.0f, 1.0f);
    g.setColour(Colours::white);
    g.setFont(14.0f);
	g.drawText(item->niceName, getMainBounds(), Justification::centred, true);



}

void GridClipUI::resized()
{
}
