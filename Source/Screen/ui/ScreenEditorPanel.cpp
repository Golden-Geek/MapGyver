/*
  ==============================================================================

	ScreenEditorPanel.cpp
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"

//EDITOR VIEW

ScreenEditorView::ScreenEditorView(Screen* screen) :
	InspectableContentComponent(screen),
	screen(screen)
{
	selectionContourColor = NORMAL_COLOR;
	GlContextHolder::getInstance()->registerOpenGlRenderer(this);
}

ScreenEditorView::~ScreenEditorView()
{
	if(GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
}

void ScreenEditorView::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	//g.fillAll(BG_COLOR);
	g.setColour(TEXT_COLOR);
	g.setFont(16);
	g.drawFittedText("Editing " + screen->niceName, getLocalBounds(), Justification::centred, 1);

	Point<int> pos = getTopLevelComponent()->getLocalPoint(this, Point<int>(0, 0));
	
}

void ScreenEditorView::newOpenGLContextCreated()
{

}

void ScreenEditorView::renderOpenGL()
{
	if (inspectable.wasObjectDeleted()) return;

	//draw rectangle of the compoennt size
	//glViewport(0, 0, getWidth(), getHeight());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, getWidth(), getHeight(), 0, 0, 1);
	
	//draw the red rectangle
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0, 0);
	glVertex2f(getWidth(), 0);
	glVertex2f(getWidth(), getHeight());
	glVertex2f(0, getHeight());
	glEnd();


}


void ScreenEditorView::openGLContextClosing()
{
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
	//nothing here
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

