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
	zoomSensitivity(3.f),
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
	setWantsKeyboardFocus(true); // Permet au composant de recevoir le focus clavier.
	addKeyListener(this);
}

ScreenEditorView::~ScreenEditorView()
{
	if (GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
	removeKeyListener(this);
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

		for (auto& s : screen->surfaces.items)
		{
			if (!s->enabled->boolValue() || s->isUILocked->boolValue()) continue;
			Array<Point2DParameter*> handles = s->getAllHandles();
			for (auto& b : handles)
			{
				bool isCorner = b == s->topLeft || b == s->topRight || b == s->bottomLeft || b == s->bottomRight;

				if (!isCorner && !s->bezierCC.enabled->boolValue()) continue;

				bool isCurrent = b == closestHandle;
				Colour c = isCurrent ? Colours::yellow : Colours::white;
				g.setColour(c.withAlpha(isCurrent ? .8f : .5f));
				Point<int> center = getPointOnScreen(b->getPoint());
				float size = isCurrent ? 10 : 5;
				g.fillEllipse(Rectangle<float>(0, 0, size, size).withCentre(center.toFloat()));
				g.setColour(c.darker());
				g.drawEllipse(Rectangle<float>(0, 0, size, size).withCentre(center.toFloat()), 1);
			}

			if (s->bezierCC.enabled->boolValue())
			{

				Array<Line<float>> handleBezierLines = {
				Line<float>(getPointOnScreen(s->topLeft->getPoint()).toFloat(), getPointOnScreen(s->handleBezierTopLeft->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->topLeft->getPoint()).toFloat(), getPointOnScreen(s->handleBezierLeftTop->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->topRight->getPoint()).toFloat(), getPointOnScreen(s->handleBezierTopRight->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->topRight->getPoint()).toFloat(), getPointOnScreen(s->handleBezierRightTop->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->bottomLeft->getPoint()).toFloat(), getPointOnScreen(s->handleBezierBottomLeft->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->bottomLeft->getPoint()).toFloat(), getPointOnScreen(s->handleBezierLeftBottom->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->bottomRight->getPoint()).toFloat(), getPointOnScreen(s->handleBezierBottomRight->getPoint()).toFloat()),
				Line<float>(getPointOnScreen(s->bottomRight->getPoint()).toFloat(), getPointOnScreen(s->handleBezierRightBottom->getPoint()).toFloat()),
				};

				g.setColour(Colours::white.withAlpha(.5f));
				for (auto& l : handleBezierLines) g.drawLine(l, 1);
			}
		}
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
	}

	panningMode = KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey);
	if (panningMode)
	{
		offsetAtMouseDown = viewOffset;
	}

	if (zoomingMode || panningMode)
	{
		closestHandle = nullptr;
		repaint();
		return;
	}

	//surface manip mode
	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = manipSurface->getAllHandles();
		posAtMouseDown.clear();
		for (auto& h : handles) posAtMouseDown.add(h->getPoint());
	}
	else if (closestHandle != nullptr)
	{
		posAtMouseDown = { closestHandle->getPoint() };

		Surface* s = ControllableUtil::findParentAs<Surface>(closestHandle);
		bool isCorner = closestHandle == s->topLeft || closestHandle == s->topRight || closestHandle == s->bottomLeft || closestHandle == s->bottomRight;
		if (isCorner)
		{
			for (auto& h : s->getBezierHandles(closestHandle)) posAtMouseDown.add(h->getPoint());
		}

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
		zoom = zoomAtMouseDown + offsetRelative.y * zoomSensitivity;
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
		Array<Point2DParameter*> handles = manipSurface->getAllHandles();
		for (int i = 0; i < handles.size(); i++) handles[i]->setPoint(posAtMouseDown[i] + offsetRelative);
	}
	else if (closestHandle != nullptr)
	{
		Array<Point2DParameter*> handles = { closestHandle };
		Surface* s = ControllableUtil::findParentAs<Surface>(closestHandle);
		bool isCorner = closestHandle == s->topLeft || closestHandle == s->topRight || closestHandle == s->bottomLeft || closestHandle == s->bottomRight;
		if (isCorner && s->bezierCC.enabled->boolValue()) handles.addArray(s->getBezierHandles(closestHandle));

		for (int i = 0; i < handles.size(); i++)
		{
			Point<float> tp = posAtMouseDown[i] + offsetRelative;

			if (i == 0 && e.mods.isCommandDown()) //only corner
			{
				Point2DParameter* th = screen->getSnapHandle(tp, closestHandle);
				if (th != nullptr) tp = th->getPoint();
			}

			handles[i]->setPoint(tp);
			if (handles[i] == closestHandle) {
				for (auto& h : overlapHandles) h->setPoint(tp);
			}
		}

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
		Array<Point2DParameter*> handles = manipSurface->getAllHandles();
		Array<UndoableAction*> actions;
		for (int i = 0; i < handles.size(); i++) actions.add(handles[i]->setUndoablePoint(posAtMouseDown[i], handles[i]->getPoint(), true));
		UndoMaster::getInstance()->performActions("Move surface", actions);

	}
	else if (closestHandle != nullptr)
	{
		Array<Point2DParameter*> handles = { closestHandle };
		Surface* s = ControllableUtil::findParentAs<Surface>(closestHandle);
		bool isCorner = closestHandle == s->topLeft || closestHandle == s->topRight || closestHandle == s->bottomLeft || closestHandle == s->bottomRight;
		if (isCorner) handles.addArray(s->getBezierHandles(closestHandle));
		handles.addArray(overlapHandles);

		Array<UndoableAction*> actions;
		for (int i = 0; i < handles.size(); i++) actions.add(handles[i]->setUndoablePoint(posAtMouseDown[i], handles[i]->getPoint(), true));
		UndoMaster::getInstance()->performActions("Move handles", actions);
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

	Init2DMatrix(getWidth(), getHeight());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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



	int fw = frameBuffer->getWidth();
	int fh = frameBuffer->getHeight();

	float fr = fw * 1.0f / fh;
	float r = getWidth() * 1.0f / getHeight();

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
	glTexCoord2f(ox, oy + hZoom); glVertex2f(tx, ty);
	glTexCoord2f(ox + rZoom, oy + hZoom); glVertex2f(tx + tw, ty);
	glTexCoord2f(ox + rZoom, oy + 1); glVertex2f(tx + tw, ty + th);
	glTexCoord2f(ox, oy + 1); glVertex2f(tx, ty + th);
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
	if (dragSourceDetails.description.getProperty("type", "") == "OnlineContentItem") return true;
	return false;
}

void ScreenEditorView::itemDragEnter(const SourceDetails& source)
{
	Media* m = nullptr;
	if (BaseItemMinimalUI<Media>* mui = dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get())) mui->item;

	setCandidateDropSurface(screen->getSurfaceAt(getRelativeMousePos()), m);
	repaint();
}

void ScreenEditorView::itemDragMove(const SourceDetails& source)
{
	Media* m = nullptr;
	if (BaseItemMinimalUI<Media>* mui = dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get())) mui->item;

	setCandidateDropSurface(screen->getSurfaceAt(getRelativeMousePos()), m);
	repaint();
}

void ScreenEditorView::itemDragExit(const SourceDetails& source)
{
	setCandidateDropSurface(nullptr);
	repaint();
}


void ScreenEditorView::itemDropped(const SourceDetails& source)
{
	if (candidateDropSurface == nullptr) return;

	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		OnlineContentItem* item = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get());
		if (item != nullptr)
		{
			if (Media* m = item->createMedia())
			{
				MediaManager::getInstance()->addItem(m);
				candidateDropSurface->media->setValueFromTarget(m);
			}
		}
	}
	else
	{
		candidateDropSurface->media->setValueFromTarget(dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get())->item);
	}

	setCandidateDropSurface(nullptr);
	repaint();
}

void ScreenEditorView::setCandidateDropSurface(Surface* s, Media* m)
{
	if (candidateDropSurface == s) return;
	if (candidateDropSurface != nullptr) candidateDropSurface->previewMedia = nullptr;

	candidateDropSurface = s;
	if (candidateDropSurface != nullptr) candidateDropSurface->previewMedia = m;
}

bool ScreenEditorView::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (key.getKeyCode() != KeyPress::leftKey && key.getKeyCode() != KeyPress::rightKey && key.getKeyCode() != KeyPress::upKey && key.getKeyCode() != KeyPress::downKey) return false;
	if (closestHandle != nullptr)
	{
		Array<Point2DParameter*> handles = { closestHandle };
		Surface* s = ControllableUtil::findParentAs<Surface>(closestHandle);
		bool isCorner = closestHandle == s->topLeft || closestHandle == s->topRight || closestHandle == s->bottomLeft || closestHandle == s->bottomRight;
		if (isCorner && s->bezierCC.enabled->boolValue()) handles.addArray(s->getBezierHandles(closestHandle));

		float xPixel = 1 / screen->screenWidth->floatValue();
		float yPixel = 1 / screen->screenHeight->floatValue();

		for (int i = 0; i < handles.size(); i++)
		{
			Point<float> tp = handles[i]->getPoint();
			if (key.getKeyCode() == KeyPress::leftKey) tp.x -= xPixel;
			else if (key.getKeyCode() == KeyPress::rightKey) tp.x += xPixel;
			else if (key.getKeyCode() == KeyPress::upKey) tp.y += yPixel;
			else if (key.getKeyCode() == KeyPress::downKey) tp.y -= yPixel;

			handles[i]->setPoint(tp);
			//if (handles[i] == closestHandle) {
			//	for (auto& h : overlapHandles) h->setPoint(tp);
			//}
		}

	}
	repaint();
	return true;
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
	setCurrentScreen(nullptr);
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
		if (screenEditorView->screen != nullptr) screenEditorView->screen->removeInspectableListener(this);
		removeChildComponent(screenEditorView.get());
		screenEditorView.reset();

	}

	if (screen != nullptr)
	{
		screenEditorView.reset(new ScreenEditorView(screen));
		addAndMakeVisible(screenEditorView.get());
		screen->addInspectableListener(this);
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

