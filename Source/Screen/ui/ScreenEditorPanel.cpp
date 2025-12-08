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
#include "ScreenEditorPanel.h"

//EDITOR VIEW

ScreenEditor::ScreenEditor() :
	OpenGLSharedRenderer(this),
	screen(nullptr),
	zoomSensitivity(3.f),
	zoomingMode(false),
	panningMode(false),
	closestHandle(nullptr),
	selectedPinMediaHandle(nullptr),
	manipSurface(nullptr),
	candidateDropSurface(nullptr),
	zoom(1),
	zoomAtMouseDown(1)
{
	setWantsKeyboardFocus(true); // Permet au composant de recevoir le focus clavier.
	addKeyListener(this);
}

ScreenEditor::~ScreenEditor()
{
	setScreen(nullptr);
	removeKeyListener(this);
}

void ScreenEditor::setScreen(Screen* s)
{
	if (screen == s) return;

	screen = s;

	

	manipSurface = nullptr;
	closestHandle = nullptr;
	selectedPinMediaHandle = nullptr;
	candidateDropSurface = nullptr;

	repaint();
	context.triggerRepaint();
}

void ScreenEditor::paint(Graphics& g)
{
	if (screen == nullptr)
	{
		g.setFont(16);
		g.setColour(NORMAL_COLOR.withAlpha(.5f));
		g.drawFittedText("Select a screen to edit", getLocalBounds(), Justification::centred, 2);
		return;
	}

	if (frameBufferRect.isEmpty()) return;

	Point<int> topLeft = getPointOnScreen(Point<float>(0, 0));
	Point<int> bottomRight = getPointOnScreen(Point<float>(1, 1));
	Rectangle<int> r(topLeft, bottomRight);

	Path p;
	p.addRectangle(0, 0, getWidth(), getHeight());
	p.setUsingNonZeroWinding(false);
	p.addRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight());

	g.setColour(BG_COLOR.darker(.5f).withAlpha(.8f));
	g.fillPath(p);

	g.setColour(NORMAL_COLOR.withAlpha(.5f));
	g.drawRect(r.toFloat(), 1);

	if (zoomingMode) return;

	Colour col = isMouseButtonDown() ? Colours::yellow : Colours::cyan;

	if (manipSurface != nullptr || candidateDropSurface != nullptr)
	{
		Surface* surface = candidateDropSurface != nullptr ? candidateDropSurface : manipSurface;

		if (candidateDropSurface != nullptr) col = Colours::purple;

		Path p = getSurfacePath(surface);
		g.setColour(col.withAlpha(.1f));
		g.fillPath(p);
		g.setColour(col.brighter(.3f));
		g.strokePath(p, PathStrokeType(2.0f));

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

			Path p = getSurfacePath(s);

			bool isInPath = p.contains(mp.toFloat());

			g.setColour(NORMAL_COLOR.withAlpha(isInPath ? .8f : .3f));
			g.strokePath(p, PathStrokeType(1.0f));

			if (isInPath)
			{
				Array<Point2DParameter*> handles = s->getAllHandles();
				for (auto& b : handles)
				{
					bool isCorner = b == s->topLeft || b == s->topRight || b == s->bottomLeft || b == s->bottomRight;
					bool isPin = b->niceName == "Position";

					if (!isCorner && !isPin && !s->bezierCC.enabled->boolValue()) continue;

					bool isCurrent = b == closestHandle;
					Colour c = isCurrent ? Colours::yellow : Colours::white;
					g.setColour(c.withAlpha(isCurrent ? .8f : .5f));
					Point<int> center = getPointOnScreen(b->getPoint());
					float size = isCurrent ? 10 : 5;
					g.fillEllipse(Rectangle<float>(0, 0, size, size).withCentre(center.toFloat()));
					g.setColour(c.darker());
					g.drawEllipse(Rectangle<float>(0, 0, size, size).withCentre(center.toFloat()), 1);
				}
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

	for (auto& s : screen->surfaces.items)
	{
		if (s->isSelected)
		{
			Path p = getSurfacePath(s);
			g.setColour(HIGHLIGHT_COLOR);
			g.strokePath(p, PathStrokeType(1.0f));
			break;
		}
	}
}

Path ScreenEditor::getSurfacePath(Surface* s)
{
	Path surfacePath;
	surfacePath.addPath(s->quadPath);

	AffineTransform at = AffineTransform::verticalFlip(1) //inverse the Y axis
		.followedBy(AffineTransform::translation(-viewOffset.x, viewOffset.y))
		.followedBy(AffineTransform::scale(frameBufferRect.getWidth() * zoom, frameBufferRect.getHeight() * zoom)) //scale to the component size
		.followedBy(AffineTransform::translation(frameBufferRect.getX(), frameBufferRect.getY())); //translate to the component position
	surfacePath.applyTransform(at);

	return surfacePath;

}

void ScreenEditor::mouseDown(const MouseEvent& e)
{
	if(screenIsLocked()) return;

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

	if (Surface* s = screen->getSurfaceAt(getRelativeMousePos())) s->selectThis();

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

		if (e.mods.isRightButtonDown()) {
			if (e.mods.isShiftDown()) {
				if (s != nullptr) {
					Pin* p = s->pinsCC.addItem();
					Point<float> pos = (e.mouseDownPosition.toFloat()) / Point<float>(frameBufferRect.getWidth(), frameBufferRect.getHeight());
					pos.y = 1 - pos.y;
					p->position->setPoint(Point<float>(pos));
				}
				return;
			}
			Pin* p = dynamic_cast<Pin*>(closestHandle->parentContainer.get());
			if (p != nullptr) {
				posAtMouseDown = { p->mediaPos->getPoint() };
				selectedPinMediaHandle = p->mediaPos;
				return;
			}
		}

		bool isCorner = closestHandle == s->topLeft || closestHandle == s->topRight || closestHandle == s->bottomLeft || closestHandle == s->bottomRight;
		if (isCorner)
		{
			for (auto& h : s->getBezierHandles(closestHandle)) posAtMouseDown.add(h->getPoint());
		}

		overlapHandles.clear();
		if (!e.mods.isShiftDown()) overlapHandles = screen->getOverlapHandles(closestHandle);
	}

}

void ScreenEditor::mouseMove(const MouseEvent& e)
{
	if (screenIsLocked()) return;

	if (zoomingMode || panningMode)
	{
		repaint();
		return;
	}

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

void ScreenEditor::mouseDrag(const MouseEvent& e)
{
	if (screenIsLocked()) return;

	Point<float> offsetRelative = (e.getOffsetFromDragStart().toFloat() * Point<float>(1, -1)) / Point<float>(frameBufferRect.getWidth(), frameBufferRect.getHeight());

	Point<int> focusScreenPoint = e.getMouseDownPosition();

	if (zoomingMode)
	{
		zoom = zoomAtMouseDown + offsetRelative.y * zoomSensitivity;
		moveScreenPointTo(focusPointAtMouseDown, focusScreenPoint);
		repaint();
		return;
	}

	if (panningMode)
	{
		viewOffset = offsetAtMouseDown - offsetRelative / zoom;
		repaint();
		return;
	}


	offsetRelative /= zoom;

	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = manipSurface->getAllHandles();
		for (int i = 0; i < handles.size(); i++) handles[i]->setPoint(posAtMouseDown[i] + offsetRelative);
	}
	else if (selectedPinMediaHandle != nullptr) {
		Point<float> tp = posAtMouseDown[0] + offsetRelative;
		selectedPinMediaHandle->setPoint(tp);
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

void ScreenEditor::mouseUp(const MouseEvent& e)
{
	if (screenIsLocked()) return;

	selectedPinMediaHandle = nullptr;
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
		for (int i = 0; i < handles.size(); i++) actions.addArray(handles[i]->setUndoablePoint(handles[i]->getPoint(), true));
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
		for (int i = 0; i < handles.size(); i++) actions.addArray(handles[i]->setUndoablePoint(handles[i]->getPoint(), true));
		UndoMaster::getInstance()->performActions("Move handles", actions);
		overlapHandles.clear();
	}

	repaint();
}

void ScreenEditor::mouseExit(const MouseEvent& e)
{
	if (screenIsLocked()) return;

	manipSurface = nullptr;
	closestHandle = nullptr;
	repaint();
}

void ScreenEditor::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
	if (screenIsLocked()) return;

	Point<float> screenPos = getRelativeMousePos();
	zoom += wheel.deltaY * zoomSensitivity / 10;
	moveScreenPointTo(screenPos, getMouseXYRelative());
	repaint();
}


Point<float> ScreenEditor::getRelativeMousePos()
{
	return getRelativeScreenPos(getMouseXYRelative());
}

Point<float> ScreenEditor::getRelativeScreenPos(Point<int> screenPos)
{
	Point<float> p = screenPos.toFloat() - frameBufferRect.getTopLeft().toFloat();
	return Point<float>(p.x / (frameBufferRect.getWidth() * zoom) + viewOffset.x, 1 - (p.y / (frameBufferRect.getHeight() * zoom) - viewOffset.y));
}

Point<int> ScreenEditor::getPointOnScreen(Point<float> pos)
{
	return frameBufferRect.getTopLeft() + Point<float>((pos.x - viewOffset.x) * (frameBufferRect.getWidth() * zoom), (1 - pos.y + viewOffset.y) * (frameBufferRect.getHeight() * zoom)).toInt();
}

void ScreenEditor::renderOpenGL()
{
	Init2DMatrix(getWidth(), getHeight());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Colour c = BG_COLOR.darker();
	//draw BG
	glBegin(GL_QUADS);
	glColor3f(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());

	Draw2DTexRect(getX(), getY(), getWidth(), getHeight());

	if (screen == nullptr || screenRef.wasObjectDeleted()) return;

	OpenGLFrameBuffer* frameBuffer = &screen->renderer->frameBuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


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

	Draw2DSubTexRect(tx, ty, tw, th, ox, oy + hZoom, ox + rZoom, oy + 1);

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
}


void ScreenEditor::openGLContextClosing()
{
}

bool ScreenEditor::screenIsLocked()
{
	return screen == nullptr || screen->isUILocked->boolValue();
}

bool ScreenEditor::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (screenIsLocked()) return false;

	if (dragSourceDetails.description.getProperty("dataType", "") == "Media") return true;
	if (dragSourceDetails.description.getProperty("type", "") == "OnlineContentItem") return true;
	return false;
}

void ScreenEditor::itemDragEnter(const SourceDetails& source)
{
	if (screenIsLocked()) return;

	Media* m = nullptr;
	if (ItemMinimalUI<Media>* mui = dynamic_cast<ItemMinimalUI<Media>*>(source.sourceComponent.get())) mui->item;

	setCandidateDropSurface(screen->getSurfaceAt(getRelativeMousePos()), m);
	repaint();
}

void ScreenEditor::itemDragMove(const SourceDetails& source)
{
	if (screenIsLocked()) return;

	Media* m = nullptr;
	if (ItemMinimalUI<Media>* mui = dynamic_cast<ItemMinimalUI<Media>*>(source.sourceComponent.get())) mui->item;

	setCandidateDropSurface(screen->getSurfaceAt(getRelativeMousePos()), m);
	repaint();
}

void ScreenEditor::itemDragExit(const SourceDetails& source)
{
	if (screenIsLocked()) return;

	setCandidateDropSurface(nullptr);
	repaint();
}


void ScreenEditor::itemDropped(const SourceDetails& source)
{
	if (screenIsLocked()) return;

	if (candidateDropSurface == nullptr) return;

	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		OnlineContentItem* item = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get());
		if (item != nullptr)
		{
			if (Media* m = item->createMedia())
			{
				MediaManager::getInstance()->addItem(m);
				candidateDropSurface->mediaParam->setValueFromTarget(m);
			}
		}
	}
	else
	{
		if (ItemMinimalUI<Media>* mui = dynamic_cast<ItemMinimalUI<Media>*>(source.sourceComponent.get()))
		{
			Media* m = mui->item;
			if (m == nullptr) return;
			candidateDropSurface->mediaParam->setValueFromTarget(m);
		}
	}

	setCandidateDropSurface(nullptr);
	repaint();
}

void ScreenEditor::setCandidateDropSurface(Surface* s, Media* m)
{
	if (screenIsLocked()) return;

	if (candidateDropSurface == s) return;
	if (candidateDropSurface != nullptr) candidateDropSurface->previewMedia = nullptr;

	candidateDropSurface = s;
	if (candidateDropSurface != nullptr) candidateDropSurface->previewMedia = m;
}

bool ScreenEditor::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (screenIsLocked()) return false;

	if (key.getTextCharacter() == 'f')
	{
		zoom = 1;
		viewOffset = Point<float>();
		repaint();
		return true;
	}

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

void ScreenEditor::moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen)
{
	Point<float> relativePosOnScreen = getRelativeScreenPos(posOnScreen);
	viewOffset += screenPos - relativePosOnScreen;
}



// 
ScreenEditorPanel::ScreenEditorPanel() :
	ShapeShifterContentComponent("Screen Editor")
{
	setWantsKeyboardFocus(true); // Permet au composant de recevoir le focus clavier.

	Engine::mainEngine->addEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);


	setScreen(ScreenManager::getInstance()->editingScreen);

	resized();
}

ScreenEditorPanel::~ScreenEditorPanel()
{
	Engine::mainEngine->removeEngineListener(this);
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
	setScreen(nullptr);


}

void ScreenEditorPanel::resized()
{
	Rectangle<int> r = getLocalBounds();
	Rectangle<int> headerRect = r.removeFromTop(20);

	if (lockPreviewUI != nullptr)
	{
		lockPreviewUI->setBounds(headerRect.removeFromRight(headerRect.getHeight()).reduced(2));
	}

	editor.setBounds(r);
}

void ScreenEditorPanel::setScreen(Screen* s)
{
	if (lockPreviewUI != nullptr) removeChildComponent(lockPreviewUI.get());

	if (editor.screen != nullptr)
	{
		editor.screen->removeInspectableListener(this);
		removeChildComponent(&editor);
	}

	editor.setScreen(s);
	
	if (editor.screen != nullptr)
	{
		editor.screen->addInspectableListener(this);
		ScreenManager::getInstance()->editingScreen = editor.screen;
		addAndMakeVisible(&editor);
	}


	setCustomName("Screen Editor " + String(editor.screen != nullptr ? " : " + editor.screen->niceName : ""));

	if (editor.screen != nullptr)
	{
		lockPreviewUI.reset(editor.screen->isUILocked->createToggle(AssetManager::getInstance()->padlockImage));
		addAndMakeVisible(lockPreviewUI.get());
	}

	resized();

}

void ScreenEditorPanel::inspectablesSelectionChanged()
{
	if (Screen* s = InspectableSelectionManager::mainSelectionManager->getInspectableAs<Screen>())
	{
		setScreen(s);
	}
}

void ScreenEditorPanel::inspectableDestroyed(Inspectable* i)
{
	if (editor.screen == i) editor.setScreen(nullptr);
}

void ScreenEditorPanel::startLoadFile()
{
	editor.setScreen(nullptr);
}
