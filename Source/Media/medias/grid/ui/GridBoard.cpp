/*
  ==============================================================================

	GridBoard.cpp
	Created: 12 Feb 2025 11:52:03am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

GridBoard::GridBoard(GridMedia* gridMedia) :
	Component("Grid Board"),
	gridMedia(gridMedia),
	inspectable(gridMedia)
{
}

GridBoard::~GridBoard()
{
}

void GridBoard::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	g.fillAll(Colours::black); // Fill the background with black
	g.setColour(Colours::white.withAlpha(.3f)); // Set the drawing color to white
	// Draw a simple grid
	int gridSize = 20; // Size of each grid cell
	for (int x = 0; x < getWidth(); x += gridSize)
	{
		g.drawLine(x, 0, x, getHeight());
	}
	for (int y = 0; y < getHeight(); y += gridSize)
	{
		g.drawLine(0, y, getWidth(), y);
	}

	//draw name of grid media
	g.setColour(TEXT_COLOR);
	g.setFont(FontOptions(16.0f, Font::bold));
	g.drawText(gridMedia->niceName, getLocalBounds(), Justification::centred, true);
}

void GridBoard::resized()
{
	if (inspectable.wasObjectDeleted()) return;
}
