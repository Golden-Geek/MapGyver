/*
  ==============================================================================

	ScreenEditorPanel.cpp
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

//EDITOR VIEW

ScreenEditorView::ScreenEditorView(Screen* screen) :
	InspectableContentComponent(screen),
	screen(screen)
{
	selectionContourColor = NORMAL_COLOR;
}

ScreenEditorView::~ScreenEditorView()
{
}

void ScreenEditorView::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	g.fillAll(BG_COLOR);
	g.setColour(TEXT_COLOR);
	g.setFont(16);
	g.drawFittedText("Editing " + screen->niceName, getLocalBounds(), Justification::centred, 1);

}


// PANEL


ScreenEditorPanel::ScreenEditorPanel(const String& name) :
	ShapeShifterContentComponent(name)
{
	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
}

ScreenEditorPanel::~ScreenEditorPanel()
{
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
}

void ScreenEditorPanel::paint(Graphics& g)
{
	if (screenEditorView == nullptr)
	{
		g.setColour(TEXT_COLOR);
		g.setFont(16);
		g.drawFittedText("Select a screen to edit it here", getLocalBounds(), Justification::centred, 1);
	}
}

void ScreenEditorPanel::resized()
{
	if (screenEditorView != nullptr)
		screenEditorView->setBounds(getLocalBounds());
}

void ScreenEditorPanel::setCurrentScreen(Screen* screen)
{
	if (screenEditorView != nullptr)
	{
		if (screenEditorView->screen == screen) return;
		removeChildComponent(screenEditorView.get());
		screenEditorView.reset();
	}

	if (screen != nullptr)
	{
		screenEditorView.reset(new ScreenEditorView(screen));
		addAndMakeVisible(screenEditorView.get());
	}

	resized();
}

void ScreenEditorPanel::inspectablesSelectionChanged()
{
	if (Screen* s = InspectableSelectionManager::mainSelectionManager->getInspectableAs<Screen>()) setCurrentScreen(s);
}

void ScreenEditorPanel::inspectableDestroyed(Inspectable* i)
{
	if (screenEditorView != nullptr && screenEditorView->screen == i) setCurrentScreen(nullptr);
}

