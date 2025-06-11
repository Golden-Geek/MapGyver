/*
  ==============================================================================

	GridMediaPanel.cpp
	Created: 12 Feb 2025 11:50:10am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

GridMediaPanel::GridMediaPanel(const String &name) :
	ShapeShifterContentComponent(name),
	currentMedia(nullptr)
{
	if(GridMedia* m = InspectableSelectionManager::mainSelectionManager->getInspectableAs<GridMedia>())
	{
		setGridMedia(m);
	}

	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
}

GridMediaPanel::~GridMediaPanel()
{
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
	setGridMedia(nullptr);
}

void GridMediaPanel::paint(Graphics& g)
{
	ShapeShifterContentComponent::paint(g);
	g.setColour(Colours::darkgrey);
	g.drawText("No Grid Media Selected", getLocalBounds(), Justification::centred, true);
}

void GridMediaPanel::resized()
{
	ShapeShifterContentComponent::resized();

	if (gridBoard != nullptr) gridBoard->setBounds(getLocalBounds());
}

void GridMediaPanel::setGridMedia(GridMedia* media)
{
	if (currentMedia == media) return;
	if (Engine::mainEngine->isClearing) return;

	if (currentMedia != nullptr)
	{
		currentMedia->removeInspectableListener(this);
	}

	if (gridBoard != nullptr)
	{
		removeChildComponent(gridBoard.get());
		gridBoard.reset();
	}

	currentMedia = media;

	if (currentMedia != nullptr)
	{
		currentMedia->addInspectableListener(this);

		gridBoard.reset(new GridBoard(currentMedia));
		addAndMakeVisible(gridBoard.get());

		setCustomName("Grid Editor : " + currentMedia->niceName);
	}
	else
	{
		setCustomName("Grid Editor");
	}

	resized();
	repaint();
}

void GridMediaPanel::inspectablesSelectionChanged()
{
	if (GridMedia* m = InspectableSelectionManager::mainSelectionManager->getInspectableAs<GridMedia>())
	{
		setGridMedia(m);
	}
}

void GridMediaPanel::inspectableDestroyed(Inspectable* inspectable)
{
	if(inspectable == currentMedia)
	{
		setGridMedia(nullptr);
	}
}
