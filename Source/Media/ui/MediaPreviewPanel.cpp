/*
  ==============================================================================

	MediaPreviewPanel.cpp
	Created: 28 Aug 2024 11:46:30am
	Author:  bkupe

  ==============================================================================
*/

#include "../MediaIncludes.h"
#include "MediaPreviewPanel.h"

using namespace juce::gl;

MediaPreview::MediaPreview() :
	OpenGLSharedRenderer(this),
	useMediaOnPreview(false),
	media(nullptr),
	webMedia(nullptr)
{

	setSize(200, 200);
}

MediaPreview::~MediaPreview()
{
	setMedia(nullptr);
}

void MediaPreview::setMedia(Media* m)
{
	if (media != nullptr)
	{
		media->removeInspectableListener(this);
		if (useMediaOnPreview) unregisterUseMedia(0);
	}
	media = m;

	if (media != nullptr)
	{


		media->addInspectableListener(this);
		if (useMediaOnPreview) registerUseMedia(0, media);
	}

	computeMediaRect();

	if (WebMedia* wm = dynamic_cast<WebMedia*>(media))
	{
		webMedia = wm;
	}
	else
	{
		webMedia = nullptr;
	}


}

void MediaPreview::paint(Graphics& g)
{
	if (!useMediaOnPreview && media != nullptr)
	{
		g.setColour(media->isBeingUsed->boolValue() ? GREEN_COLOR : media->usedTargets.isEmpty() ? NORMAL_COLOR : BLUE_COLOR);
		g.drawRoundedRectangle(getLocalBounds().toFloat(), 4, 1);
	}
}

void MediaPreview::resized()
{
	computeMediaRect();
}

void MediaPreview::computeMediaRect()
{
	if (media == nullptr)
	{
		mediaRect = Rectangle<int>();
		return;
	}

	Point<int> size = media->getMediaSize();

	int fw = size.x;
	int fh = size.y;

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

	mediaRect = Rectangle<int>(tx, ty, tw, th);
}

void MediaPreview::renderOpenGL()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Init2DMatrix(getWidth(), getHeight());

	//draw BG
	glColor4f(0, 0, 0, .3f);
	Draw2DRect(0, 0, getWidth(), getHeight());


	if (media == nullptr || mediaRect.isEmpty()) return;

	////draw frameBuffer
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, media->getTextureID());

	Draw2DTexRect(mediaRect.getX(), mediaRect.getY(), mediaRect.getWidth(), mediaRect.getHeight());

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

void MediaPreview::openGLContextClosing()
{
}


void MediaPreview::inspectableDestroyed(Inspectable* i)
{
	if (i == media) setMedia(nullptr);
}

void MediaPreview::mouseDown(const MouseEvent& e)
{
	if (webMedia == nullptr) return;
	webMedia->sendMouseDown(e, mediaRect);
}

void MediaPreview::mouseUp(const MouseEvent& e)
{
	if (webMedia == nullptr) return;
	webMedia->sendMouseUp(e, mediaRect);
}

void MediaPreview::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
	if (webMedia == nullptr) return;
	webMedia->sendMouseWheelMove(e, wheel);
}

void MediaPreview::mouseMove(const MouseEvent& e)
{
	if (webMedia == nullptr) return;
	webMedia->sendMouseMove(e, mediaRect);
}

void MediaPreview::mouseDrag(const MouseEvent& e)
{
	if (webMedia == nullptr) return;
	webMedia->sendMouseDrag(e, mediaRect);
}

bool MediaPreview::keyPressed(const KeyPress& key)
{
	if (webMedia == nullptr) return false;

	webMedia->sendKeyPressed(key);

	return false;
}


MediaPreviewPanel::MediaPreviewPanel() :
	ShapeShifterContentComponent("Media Preview"),
	lockPreview("Lock", "If enabled, the media preview will not change when selecting different media", false)
{

	lockPreviewUI.reset(lockPreview.createToggle(AssetManager::getInstance()->padlockImage));
	addAndMakeVisible(lockPreviewUI.get());

	preview.useMediaOnPreview = true;
	addAndMakeVisible(preview);
	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);
}


//implement all methods
MediaPreviewPanel::~MediaPreviewPanel()
{
	InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);
}

void MediaPreviewPanel::paint(Graphics& g)
{
	g.fillAll(BG_COLOR);
}

void MediaPreviewPanel::resized()
{
	Rectangle<int> r = getLocalBounds();
	Rectangle<int> hr = r.removeFromTop(20);
	preview.setBounds(r);
	lockPreviewUI->setBounds(hr.removeFromRight(20).reduced(2));
}

void MediaPreviewPanel::checkAndAssignPreview(InspectableSelectionManager* selectionManager)
{
	if (selectionManager == nullptr) selectionManager = InspectableSelectionManager::mainSelectionManager;

	if (Media* m = selectionManager->getInspectableAs<Media>())
	{
		if (m->doNotPreview->boolValue())
		{
			preview.setMedia(nullptr);
			setCustomName("Media Preview : (No Preview)");
			return;
		}

		preview.setMedia(m);
		setCustomName("Media Preview : " + m->niceName);
	}
}

void MediaPreviewPanel::newMessage(const Parameter::ParameterEvent& e)
{
	//if lock changed, update preview lock state
	if (e.parameter == &lockPreview)
	{
		if (!lockPreview.boolValue())
		{
			checkAndAssignPreview();
		}
	}
}

void MediaPreviewPanel::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (lockPreview.boolValue()) return;
	if (e.type == InspectableSelectionManager::SelectionEvent::SELECTION_CHANGED)
	{
		checkAndAssignPreview(e.selectionManager);
	}
}
