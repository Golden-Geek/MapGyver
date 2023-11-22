/*
  ==============================================================================

	ScreenEditorPanel.cpp
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"

//EDITOR VIEW

ScreenEditorView::ScreenEditorView(Screen* screen) :
	InspectableContentComponent(screen),
	screen(screen),
	zoomSensitivity(1.5f),
	zoomingMode(false),
	panningMode(false),
	closestHandle(nullptr),
	manipSurface(nullptr),
	candidateDropSurface(nullptr),
	zoom(1),
	zoomAtMouseDown(1)
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
	if (frameBufferRect.isEmpty()) return;

	if (zoomingMode)
	{
		return;
	}

	Colour col = isMouseButtonDown() ? Colours::yellow : Colours::cyan;

	if (manipSurface != nullptr || candidateDropSurface != nullptr)
	{
		Surface* surface = candidateDropSurface != nullptr ? candidateDropSurface : manipSurface;

		if (candidateDropSurface != nullptr) col = Colours::purple;

		Path surfacePath;
		surfacePath.addPath(surface->quadPath);

		AffineTransform at = AffineTransform::verticalFlip(1) //inverse the Y axis
			.followedBy(AffineTransform::translation(-viewOffset.x, viewOffset.y))
			.followedBy(AffineTransform::scale(frameBufferRect.getWidth() * zoom, frameBufferRect.getHeight() * zoom)) //scale to the component size
			.followedBy(AffineTransform::translation(frameBufferRect.getX(), frameBufferRect.getY())); //translate to the component position
		surfacePath.applyTransform(at);


		g.setColour(col.withAlpha(.1f));
		g.fillPath(surfacePath);
		g.setColour(col.brighter(.3f));
		g.strokePath(surfacePath, PathStrokeType(2.0f));

		if (manipSurface != nullptr)
		{
			g.drawLine(Line<float>(getPointOnScreen(manipSurface->topLeft->getPoint()).toFloat(), getPointOnScreen(manipSurface->bottomRight->getPoint()).toFloat()), 2.0f);
			g.drawLine(Line<float>(getPointOnScreen(manipSurface->topRight->getPoint()).toFloat(), getPointOnScreen(manipSurface->bottomLeft->getPoint()).toFloat()), 2.0f);
		}
	}
	else if (closestHandle != nullptr)
	{
		Point<int> mp = getMouseXYRelative();
		Point<int> hp = getPointOnScreen(closestHandle->getPoint());// frameBufferRect.getRelativePoint(closestHandle->x, 1 - closestHandle->y);

		float angle = mp.getAngleToPoint(hp);

		Point<float> a1 = Point<float>(cosf(angle), sinf(angle)) * 20;
		Point<float> a2 = Point<float>(cosf(angle + MathConstants<float>::pi), sinf(angle + MathConstants<float>::pi)) * 20;

		Path p;
		p.startNewSubPath(mp.toFloat() + a1);
		p.lineTo(mp.toFloat() + a2);
		p.lineTo(hp.toFloat());
		p.closeSubPath();

		g.setColour(col.withAlpha(.5f));
		g.fillPath(p);
	}
}


void ScreenEditorView::mouseDown(const MouseEvent& e)
{
	zoomingMode = e.mods.isCommandDown() && KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey);
	if (zoomingMode)
	{
		zoomAtMouseDown = zoom;
		offsetAtMouseDown = viewOffset;
		focusPointAtMouseDown = getRelativeMousePos();
		return;
	}

	panningMode = KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey);
	if (panningMode)
	{
		offsetAtMouseDown = viewOffset;
		return;
	}


	//surface manip mode
	if (manipSurface != nullptr)
	{
		posAtMouseDown = { manipSurface->topLeft->getPoint(), manipSurface->topRight->getPoint(), manipSurface->bottomLeft->getPoint(), manipSurface->bottomRight->getPoint() };

	}
	else if (closestHandle != nullptr)
	{
		posAtMouseDown = { closestHandle->getPoint() };
		overlapHandles.clear();
		if (e.mods.isShiftDown()) overlapHandles = screen->getOverlapHandles(closestHandle);
	}

}

void ScreenEditorView::mouseMove(const MouseEvent& e)
{
	if (zoomingMode || panningMode) return;

	if (e.mods.isAltDown())
	{
		closestHandle = nullptr;
		manipSurface = screen->getSurfaceAt(getRelativeMousePos());
	}
	else if (!e.mods.isLeftButtonDown())
	{
		manipSurface = nullptr;
		closestHandle = screen->getClosestHandle(getRelativeMousePos());
	}
	repaint();

}

void ScreenEditorView::mouseDrag(const MouseEvent& e)
{
	Point<float> offsetRelative = (e.getOffsetFromDragStart().toFloat() * Point<float>(1, -1)) / Point<float>(frameBufferRect.getWidth(), frameBufferRect.getHeight());

	Point<int> focusScreenPoint = e.getMouseDownPosition();

	if (zoomingMode)
	{
		zoom = zoomAtMouseDown + offsetRelative.getDistanceFromOrigin() * (offsetRelative.y > 0 ? 1 : -1) * zoomSensitivity;
		moveScreenPointTo(focusPointAtMouseDown, focusScreenPoint);
		return;
	}

	if (panningMode)
	{
		viewOffset = offsetAtMouseDown - offsetRelative / zoom;
		return;
	}


	offsetRelative /= zoom;

	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = { manipSurface->topLeft, manipSurface->topRight, manipSurface->bottomLeft, manipSurface->bottomRight };


		for (int i = 0; i < handles.size(); i++) handles[i]->setPoint(posAtMouseDown[i] + offsetRelative);
	}
	else if (closestHandle != nullptr)
	{
		Point<float> tp = posAtMouseDown[0] + offsetRelative;
		if (e.mods.isCommandDown())
		{
			Point2DParameter* th = screen->getSnapHandle(tp, closestHandle);
			if (th != nullptr) tp = th->getPoint();
		}

		closestHandle->setPoint(tp);
		for (auto& h : overlapHandles) h->setPoint(tp);

	}

	repaint();
}

void ScreenEditorView::mouseUp(const MouseEvent& e)
{
	if (zoomingMode)
	{
		zoomingMode = false;
		return;
	}

	if (panningMode)
	{
		panningMode = false;
		return;
	}

	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = { manipSurface->topLeft, manipSurface->topRight, manipSurface->bottomLeft, manipSurface->bottomRight };
		Array<UndoableAction*> actions;
		for (int i = 0; i < handles.size(); i++) actions.add(handles[i]->setUndoablePoint(posAtMouseDown[i], handles[i]->getPoint(), true));
		UndoMaster::getInstance()->performActions("Move surface", actions);

	}
	else if (closestHandle != nullptr)
	{

		if (overlapHandles.size() > 0)
		{
			Array<UndoableAction*> actions;
			actions.add(closestHandle->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint(), true));
			for (auto& h : overlapHandles) actions.add(h->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint(), true));
			UndoMaster::getInstance()->performActions("Move handles", actions);
		}
		else closestHandle->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint());
		overlapHandles.clear();
	}
	repaint();
}

void ScreenEditorView::mouseExit(const MouseEvent& e)
{
	manipSurface = nullptr;
	closestHandle = nullptr;
	repaint();
}


Point<float> ScreenEditorView::getRelativeMousePos()
{
	return getRelativeScreenPos(getMouseXYRelative());
}

Point<float> ScreenEditorView::getRelativeScreenPos(Point<int> screenPos)
{
	Point<float> p = screenPos.toFloat() - frameBufferRect.getTopLeft().toFloat();
	return Point<float>(p.x / (frameBufferRect.getWidth() * zoom) + viewOffset.x, 1 - (p.y / (frameBufferRect.getHeight() * zoom) - viewOffset.y));
}

Point<int> ScreenEditorView::getPointOnScreen(Point<float> pos)
{
	return frameBufferRect.getTopLeft() + Point<float>((pos.x - viewOffset.x) * (frameBufferRect.getWidth() * zoom), (1 - pos.y + viewOffset.y) * (frameBufferRect.getHeight() * zoom)).toInt();
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

	Colour c = BG_COLOR.darker();
	//draw BG
	glBegin(GL_QUADS);
	glColor3f(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(0, 1); glVertex2f(0, getHeight());
	glTexCoord2f(1, 1); glVertex2f(getWidth(), getHeight());
	glTexCoord2f(1, 0); glVertex2f(getWidth(), 0);
	glEnd();
	glGetError();


	//draw frameBuffer
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

	frameBufferRect = Rectangle<int>(tx, ty, tw, th);

	float rZoom = 1 / zoom;
	float hZoom = 1 - 1 / zoom;

	float ox = viewOffset.x;
	float oy = viewOffset.y;


	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(ox, oy + 1); glVertex2f(tx, ty);
	glTexCoord2f(ox + rZoom, oy + 1); glVertex2f(tx + tw, ty);
	glTexCoord2f(ox + rZoom, oy + hZoom); glVertex2f(tx + tw, ty + th);
	glTexCoord2f(ox, oy + hZoom); glVertex2f(tx, ty + th);
	glEnd();
	glGetError();

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
}


void ScreenEditorView::openGLContextClosing()
{
}

bool ScreenEditorView::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (dragSourceDetails.description.getProperty("dataType", "") == "Media") return true;

}



void ScreenEditorView::itemDragEnter(const SourceDetails& source)
{
	if (BaseItemMinimalUI<Media>* m = dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get()))
	{
		candidateDropSurface = screen->getSurfaceAt(getRelativeMousePos());
		repaint();
	}
}

void ScreenEditorView::itemDragMove(const SourceDetails& source)
{
	if (BaseItemMinimalUI<Media>* m = dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get()))
	{
		candidateDropSurface = screen->getSurfaceAt(getRelativeMousePos());
		repaint();
	}
}

void ScreenEditorView::itemDragExit(const SourceDetails& source)
{
	candidateDropSurface = nullptr;
	repaint();
}

void ScreenEditorView::itemDropped(const SourceDetails& source)
{
	if (candidateDropSurface == nullptr) return;
	candidateDropSurface->media->setValueFromTarget(dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get())->item);
	candidateDropSurface = nullptr;
	repaint();
}

void ScreenEditorView::moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen)
{
	Point<float> relativePosOnScreen = getRelativeScreenPos(posOnScreen);
	viewOffset += screenPos - relativePosOnScreen;
}


// PANEL


ScreenEditorPanel::ScreenEditorPanel(const String& name) :
	ShapeShifterContentComponent(name)
{
	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
	if (ScreenManager::getInstance()->editingScreen != nullptr) setCurrentScreen(ScreenManager::getInstance()->editingScreen);
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

	ScreenManager::getInstance()->editingScreen = screen;

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

