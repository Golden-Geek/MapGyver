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
	if (GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
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

	OpenGLFrameBuffer* frameBuffer = &screen->renderer->frameBuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, getWidth(), getHeight(), 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, frameBuffer->getTextureID());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	int fw = frameBuffer->getWidth();
	int fh = frameBuffer->getHeight();

	float fr = fw * 1.0f / fh;
	float r = getWidth() * 1.0f / getHeight();

	DBG(fr << " " << r);
	int w = getWidth();
	int h = getHeight();

	int tw = w;
	int th = h;
	if (fr > r) th = getWidth() / fr; 
	else  tw = getHeight() * fr;

	int tx = (w - tw) / 2;
	int ty = (h - th) / 2;


	//glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(tx, ty);
	glTexCoord2f(1, 1); glVertex2f(tx + tw, ty);
	glTexCoord2f(1, 0); glVertex2f(tx + tw, ty + th);
	glTexCoord2f(0, 0); glVertex2f(tx, ty + th);
	glEnd();
	glGetError();

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
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

