
#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"
#include "SurfaceEditorPanel.h"

using namespace juce::gl;

//EDITOR VIEW
SurfaceEditorView::SurfaceEditorView(Surface* surface) :
	InspectableContentComponent(surface),
	surface(surface),
	zoomSensitivity(3.f),
	zoomingMode(false),
	panningMode(false),
	zoom(1),
	zoomAtMouseDown(1),
	focusComp(this),
	updatingFocus(false)
{
	selectionContourColor = NORMAL_COLOR;
	GlContextHolder::getInstance()->registerOpenGlRenderer(this, 3);
	setWantsKeyboardFocus(true); // Permet au composant de recevoir le focus clavier.
	addKeyListener(this);

	addAndMakeVisible(focusComp);
	updateFocus();

	surface->addAsyncContainerListener(this);
}

SurfaceEditorView::~SurfaceEditorView()
{
	if (GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
	removeKeyListener(this);

	if(!inspectable.wasObjectDeleted()) surface->removeAsyncContainerListener(this);

}

void SurfaceEditorView::resized()
{
	Media* media = surface->getMedia();
	if (media == nullptr) return;

	Point<int> mediaSize = media->getMediaSize();
	int fw = mediaSize.x;
	int fh = mediaSize.y;

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

	updateFocus();
}

void SurfaceEditorView::visibilityChanged()
{
	updateFocus();
}

void SurfaceEditorView::paint(Graphics& g)
{
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

	//draw softedge gradient feedback if they are not 0
	Point<int> tl = focusComp.getBounds().getTopLeft();
	Point<int> br = focusComp.getBounds().getBottomRight();
	Point<int> softTL = focusComp.getBounds().getRelativePoint(surface->softEdgeLeft->floatValue(), surface->softEdgeTop->floatValue());
	Point<int> softBR = focusComp.getBounds().getRelativePoint(1 - surface->softEdgeRight->floatValue(), 1 - surface->softEdgeBottom->floatValue());

	//draw soft edge zones
	if (surface->softEdgeTop->floatValue() > 0)
	{
		g.setColour(BLUE_COLOR.withAlpha(.2f));
		g.fillRect(tl.x, tl.y, br.x - tl.x, softTL.y - tl.y);
	}

	if (surface->softEdgeBottom->floatValue() > 0)
	{
		g.setColour(BLUE_COLOR.withAlpha(.2f));
		g.fillRect(tl.x, softBR.y, br.x - tl.x, br.y - softBR.y);
	}

	if (surface->softEdgeLeft->floatValue() > 0)
	{
		g.setColour(BLUE_COLOR.withAlpha(.2f));
		g.fillRect(tl.x, tl.y, softTL.x - tl.x, br.y - tl.y);
	}

	if (surface->softEdgeRight->floatValue() > 0)
	{
		g.setColour(BLUE_COLOR.withAlpha(.2f));
		g.fillRect(softBR.x, tl.y, br.x - softBR.x, br.y - tl.y);
	}
	
	updateFocus();
}

void SurfaceEditorView::newOpenGLContextCreated()
{

}

void SurfaceEditorView::renderOpenGL()
{
	if (inspectable.wasObjectDeleted()) return;

	Media* media = surface->getMedia();
	if (media == nullptr) return;

	GLint texID = media->getTextureID();
	Point<int> mediaSize = media->getMediaSize();
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
	glBindTexture(GL_TEXTURE_2D, texID);


	float rZoom = 1 / zoom;
	float hZoom = 1 - 1 / zoom;

	float ox = viewOffset.x;
	float oy = viewOffset.y;


	float tx = frameBufferRect.getX();
	float ty = frameBufferRect.getY();
	float tw = frameBufferRect.getWidth();
	float th = frameBufferRect.getHeight();

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


void SurfaceEditorView::openGLContextClosing()
{
}

bool SurfaceEditorView::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (dragSourceDetails.description.getProperty("dataType", "") == "Media") return true;
	if (dragSourceDetails.description.getProperty("type", "") == "OnlineContentItem") return true;
	return false;
}


void SurfaceEditorView::itemDropped(const SourceDetails& source)
{
	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		OnlineContentItem* item = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get());
		if (item != nullptr)
		{
			if (Media* m = item->createMedia())
			{
				MediaManager::getInstance()->addItem(m);
				surface->media->setValueFromTarget(m);
			}
		}
	}
	else
	{
		surface->media->setValueFromTarget(dynamic_cast<BaseItemMinimalUI<Media>*>(source.sourceComponent.get())->item);
	}

	repaint();
}

void SurfaceEditorView::mouseDown(const MouseEvent& e)
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
		repaint();
		return;
	}
}

void SurfaceEditorView::mouseMove(const MouseEvent& e)
{
	repaint();
}

void SurfaceEditorView::mouseDrag(const MouseEvent& e)
{
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

}

void SurfaceEditorView::mouseUp(const MouseEvent& e)
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

	repaint();
}


void SurfaceEditorView::mouseExit(const MouseEvent& e)
{
	repaint();
}

void SurfaceEditorView::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
	Point<float> screenPos = getRelativeMousePos();
	zoom += wheel.deltaY * zoomSensitivity / 10;
	moveScreenPointTo(screenPos, getMouseXYRelative());
}


bool SurfaceEditorView::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (key.getTextCharacter() == 'f')
	{
		zoom = 1;
		viewOffset = Point<float>();
		repaint();
		return true;
	}

	return false;
}

void SurfaceEditorView::updateFocus()
{
	updatingFocus = true;
	Rectangle<int> r(getPointOnScreen(Point<float>(surface->cropLeft->floatValue(), 1 - surface->cropTop->floatValue())), getPointOnScreen(Point<float>(1 - surface->cropRight->floatValue(), surface->cropBottom->floatValue())));
	focusComp.setBounds(r);
	updatingFocus = false;
}

void SurfaceEditorView::childBoundsChanged(Component* child)
{
	if (child == &focusComp && !updatingFocus)
	{
		Point<float> tl = getRelativeScreenPos(focusComp.getBounds().getTopLeft());
		Point<float> br = getRelativeScreenPos(focusComp.getBounds().getBottomRight());
		surface->cropLeft->setValue(tl.x);
		surface->cropTop->setValue(1 - tl.y);
		surface->cropRight->setValue(1 - br.x);
		surface->cropBottom->setValue(br.y);
	}
}

void SurfaceEditorView::newMessage(const ContainerAsyncEvent& e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		if (e.targetControllable == surface->cropLeft || e.targetControllable == surface->cropTop || e.targetControllable == surface->cropRight || e.targetControllable == surface->cropBottom)
		{
			updateFocus();
		}
		else if (e.targetControllable == surface->softEdgeBottom || e.targetControllable == surface->softEdgeTop || e.targetControllable == surface->softEdgeLeft || e.targetControllable == surface->softEdgeRight)
		{
			repaint();
		}
	}
}


Point<float> SurfaceEditorView::getRelativeMousePos()
{
	return getRelativeScreenPos(getMouseXYRelative());
}

Point<float> SurfaceEditorView::getRelativeScreenPos(Point<int> screenPos)
{
	Point<float> p = screenPos.toFloat() - frameBufferRect.getTopLeft().toFloat();
	return Point<float>(p.x / (frameBufferRect.getWidth() * zoom) + viewOffset.x, 1 - (p.y / (frameBufferRect.getHeight() * zoom) - viewOffset.y));
}

Point<int> SurfaceEditorView::getPointOnScreen(Point<float> pos)
{
	return frameBufferRect.getTopLeft() + Point<float>((pos.x - viewOffset.x) * (frameBufferRect.getWidth() * zoom), (1 - pos.y + viewOffset.y) * (frameBufferRect.getHeight() * zoom)).toInt();
}



void SurfaceEditorView::moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen)
{
	Point<float> relativePosOnScreen = getRelativeScreenPos(posOnScreen);
	viewOffset += screenPos - relativePosOnScreen;
}

// PANEL
SurfaceEditorPanel::SurfaceEditorPanel(const String& name) :
	ShapeShifterContentComponent(name)
{
	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
}

SurfaceEditorPanel::~SurfaceEditorPanel()
{
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
	setCurrentSurface(nullptr);
}

void SurfaceEditorPanel::paint(Graphics& g)
{
	//nothing here
	if (surfaceEditorView == nullptr)
	{
		g.setColour(TEXT_COLOR);
		g.setFont(16);
		g.drawFittedText("Select a surface to edit it here", getLocalBounds(), Justification::centred, 1);
	}
}

void SurfaceEditorPanel::resized()
{
	if (surfaceEditorView != nullptr)
		surfaceEditorView->setBounds(getLocalBounds());

}

void SurfaceEditorPanel::setCurrentSurface(Surface* surface)
{
	if (surfaceEditorView != nullptr)
	{
		if (surfaceEditorView->surface == surface) return;
		if (surfaceEditorView->surface != nullptr) surfaceEditorView->surface->removeInspectableListener(this);
		removeChildComponent(surfaceEditorView.get());
		surfaceEditorView.reset();

	}

	if (surface != nullptr)
	{
		surfaceEditorView.reset(new SurfaceEditorView(surface));
		addAndMakeVisible(surfaceEditorView.get());
		surface->addInspectableListener(this);
	}

	resized();
	if(surface != nullptr) surfaceEditorView->repaint();
}

void SurfaceEditorPanel::inspectablesSelectionChanged()
{
	if (Surface* s = InspectableSelectionManager::mainSelectionManager->getInspectableAs<Surface>()) setCurrentSurface(s);
}

void SurfaceEditorPanel::inspectableDestroyed(Inspectable* i)
{
	if (surfaceEditorView != nullptr && surfaceEditorView->surface == i) setCurrentSurface(nullptr);
}

SurfaceEditorView::FocusComp::FocusComp(SurfaceEditorView* view) :
	ResizableBorderComponent(this, nullptr)
{
}

SurfaceEditorView::FocusComp::~FocusComp()
{
}

void SurfaceEditorView::FocusComp::paint(Graphics& g)
{
	g.setColour(YELLOW_COLOR);
	g.drawRect(getLocalBounds(), 1);
}

void SurfaceEditorView::FocusComp::resized()
{
}
