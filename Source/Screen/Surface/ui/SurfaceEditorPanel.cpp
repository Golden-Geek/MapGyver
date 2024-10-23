
#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"
#include "SurfaceEditorPanel.h"

using namespace juce::gl;

//EDITOR VIEW
SurfaceEditorPanel::SurfaceEditorPanel() :
	ShapeShifterContentComponent("Surface Editor"),
	OpenGLSharedRenderer(this),
	surface(nullptr),
	zoomSensitivity(3.f),
	zoomingMode(false),
	panningMode(false),
	zoom(1),
	zoomAtMouseDown(1),
	focusComp(this),
	updatingFocus(false)
{
	setWantsKeyboardFocus(true); // Permet au composant de recevoir le focus clavier.
	addKeyListener(this);

	addAndMakeVisible(focusComp);
	updateFocus();

	Engine::mainEngine->addEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);

}

SurfaceEditorPanel::~SurfaceEditorPanel()
{
	Engine::mainEngine->removeEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);

	if (GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
	removeKeyListener(this);

	setSurface(nullptr);

}

void SurfaceEditorPanel::setSurface(Surface* s)
{
	if (surface == s) return;

	if (surface != nullptr)
	{
		surface->removeAsyncContainerListener(this);
		surface->removeInspectableListener(this);
	}

	surface = s;
	surfaceRef = s;

	if (surface != nullptr)
	{
		surface->addAsyncContainerListener(this);
		surface->addInspectableListener(this);
	}

	resized();
	repaint();
}


void SurfaceEditorPanel::resized()
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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

void SurfaceEditorPanel::visibilityChanged()
{
	updateFocus();
}

void SurfaceEditorPanel::paint(Graphics& g)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted())
	{
		ShapeShifterContentComponent::paint(g);
		g.setColour(TEXT_COLOR);
		g.setFont(16);
		g.drawFittedText("Select a surface to edit it here", getLocalBounds(), Justification::centred, 1);
		return;
	}

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


void SurfaceEditorPanel::renderOpenGL()
{

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

	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

	Media* media = surface->getMedia();
	if (media == nullptr) return;

	GLint texID = media->getTextureID();
	Point<int> mediaSize = media->getMediaSize();

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


void SurfaceEditorPanel::openGLContextClosing()
{
}

bool SurfaceEditorPanel::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return false;

	if (dragSourceDetails.description.getProperty("dataType", "") == "Media") return true;
	if (dragSourceDetails.description.getProperty("type", "") == "OnlineContentItem") return true;
	return false;
}


void SurfaceEditorPanel::itemDropped(const SourceDetails& source)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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

void SurfaceEditorPanel::mouseDown(const MouseEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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

void SurfaceEditorPanel::mouseMove(const MouseEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;
	repaint();
}

void SurfaceEditorPanel::mouseDrag(const MouseEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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

void SurfaceEditorPanel::mouseUp(const MouseEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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


void SurfaceEditorPanel::mouseExit(const MouseEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;
	repaint();
}

void SurfaceEditorPanel::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

	Point<float> screenPos = getRelativeMousePos();
	zoom += wheel.deltaY * zoomSensitivity / 10;
	moveScreenPointTo(screenPos, getMouseXYRelative());
}


bool SurfaceEditorPanel::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return false;

	if (key.getTextCharacter() == 'f')
	{
		zoom = .95f;
		viewOffset = Point<float>();
		repaint();
		return true;
	}

	return false;
}

void SurfaceEditorPanel::updateFocus()
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

	updatingFocus = true;
	Rectangle<int> r(getPointOnScreen(Point<float>(surface->cropLeft->floatValue(), 1 - surface->cropTop->floatValue())), getPointOnScreen(Point<float>(1 - surface->cropRight->floatValue(), surface->cropBottom->floatValue())));
	focusComp.setBounds(r);
	updatingFocus = false;
}

void SurfaceEditorPanel::childBoundsChanged(Component* child)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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

void SurfaceEditorPanel::newMessage(const ContainerAsyncEvent& e)
{
	if (surface == nullptr || surfaceRef.wasObjectDeleted()) return;

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


Point<float> SurfaceEditorPanel::getRelativeMousePos()
{
	return getRelativeScreenPos(getMouseXYRelative());
}

Point<float> SurfaceEditorPanel::getRelativeScreenPos(Point<int> screenPos)
{
	Point<float> p = screenPos.toFloat() - frameBufferRect.getTopLeft().toFloat();
	return Point<float>(p.x / (frameBufferRect.getWidth() * zoom) + viewOffset.x, 1 - (p.y / (frameBufferRect.getHeight() * zoom) - viewOffset.y));
}

Point<int> SurfaceEditorPanel::getPointOnScreen(Point<float> pos)
{
	return frameBufferRect.getTopLeft() + Point<float>((pos.x - viewOffset.x) * (frameBufferRect.getWidth() * zoom), (1 - pos.y + viewOffset.y) * (frameBufferRect.getHeight() * zoom)).toInt();
}



void SurfaceEditorPanel::moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen)
{
	Point<float> relativePosOnScreen = getRelativeScreenPos(posOnScreen);
	viewOffset += screenPos - relativePosOnScreen;
}

void SurfaceEditorPanel::inspectablesSelectionChanged()
{
	if (Surface* s = InspectableSelectionManager::mainSelectionManager->getInspectableAs<Surface>()) setSurface(s);
}

void SurfaceEditorPanel::inspectableDestroyed(Inspectable* i)
{
	if (surface == i) setSurface(nullptr);
}

void SurfaceEditorPanel::startLoadFile()
{
	setSurface(nullptr);
	repaint();

}


SurfaceEditorPanel::FocusComp::FocusComp(SurfaceEditorPanel* view) :
	ResizableBorderComponent(this, nullptr)
{
}

SurfaceEditorPanel::FocusComp::~FocusComp()
{
}

void SurfaceEditorPanel::FocusComp::paint(Graphics& g)
{
	g.setColour(YELLOW_COLOR);
	g.drawRect(getLocalBounds(), 1);
}

void SurfaceEditorPanel::FocusComp::resized()
{
}
