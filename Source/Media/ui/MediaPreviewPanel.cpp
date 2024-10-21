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
	OpenGLSharedRenderer(this, true),
	useMediaOnPreview(false),
	media(nullptr)
{

	setSize(200, 200);
}

MediaPreview::~MediaPreview()
{
	unregisterRenderer();
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
		if(useMediaOnPreview) registerUseMedia(0, media);
		if (!context.isAttached()) registerRenderer(50);
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

void MediaPreview::renderOpenGL()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Init2DMatrix(getWidth(), getHeight());

	//draw BG
	glColor4f(0, 0, 0, .3f);
	Draw2DRect(0, 0, getWidth(), getHeight());


	if (media == nullptr) return;
	////draw frameBuffer
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, media->getTextureID());


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


	Draw2DTexRect(tx, ty, tw, th);

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


MediaPreviewPanel::MediaPreviewPanel() :
	ShapeShifterContentComponent("Media Preview")
{
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
}

void MediaPreviewPanel::resized()
{
	preview.setBounds(getLocalBounds());
}

void MediaPreviewPanel::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == InspectableSelectionManager::SelectionEvent::SELECTION_CHANGED)
	{
		//if selection is media, then set media
		if (Media* m = e.selectionManager->getInspectableAs<Media>())
		{
			preview.setMedia(m);
			setCustomName("Media Preview : " + m->niceName);
		}
	}
}
