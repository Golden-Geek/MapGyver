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

}

void GridBoard::resized()
{
	if (inspectable.wasObjectDeleted()) return;
}
