/*
  ==============================================================================

	fListMedia.cpp
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaListMedia.h"

MediaListMedia::MediaListMedia(var params) :
	Media(getTypeString(), params, true)
{
	saveAndLoadRecursiveData = true;

	index = mediaParams.addIntParameter("Index", "Index of the media to use", 1, 1);
	addChildControllableContainer(&listManager);
	defaultTransitionTime = mediaParams.addFloatParameter("Default transition time", "Default transition time in seconds when not specified in the item", 1, 0);

	alwaysRedraw = true;
}

MediaListMedia::~MediaListMedia()
{
}



void MediaListMedia::updateMediaLoads()
{
	GenericScopedLock lock(listManager.items.getLock());

	int mIndex = index->intValue() - 1;
	if (mIndex < 0 || mIndex >= listManager.items.size())
	{
		for (auto& item : listManager.items)
		{
			item->unload(0);
		}
		return;
	}

	MediaListItem* currentItem = listManager.items[mIndex];
	float transitionTime = currentItem->transitionTime->enabled ? currentItem->transitionTime->floatValue() : defaultTransitionTime->getValue();
	for (auto& item : listManager.items)
	{
		if (item == currentItem)
			item->load(transitionTime);
		else
			item->unload(transitionTime);
	}
}

void MediaListMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (c == index)
	{
		updateMediaLoads();
	}
}


void MediaListMedia::preRenderGLInternal()
{
	for (auto& i : listManager.items)
	{
		if (i->weight->floatValue() == 0.f) continue;
		i->media->renderOpenGLMedia();
	}
}

void MediaListMedia::renderGLInternal()
{
	if (isClearing) return;

	for (auto& item : listManager.items)
	{
		item->process();
	}

	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, 1);
	Draw2DRect(0, 0, width->intValue(), height->intValue());

	GenericScopedLock lock(listManager.items.getLock());
	for (auto& i : listManager.items)
	{
		Media* m = i->media;
		if (m == nullptr) continue;

		float w = i->weight->floatValue();

		if (w == 0.f) continue;

		ShaderMedia* shaderMedia = i->shaderMedia.get();
		if (shaderMedia != nullptr && shaderMedia->shaderLoaded->boolValue())
		{
			//later implement shader transition
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, i->media->frameBuffer.getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Rectangle<int> mediaRect = i->media->getMediaRect(Rectangle<int>(0, 0, width->intValue(), height->intValue()));
		glColor4f(1, 1, 1, w);

		Draw2DTexRect(mediaRect.getX(), mediaRect.getY(), mediaRect.getWidth(), mediaRect.getHeight());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void MediaListMedia::afterLoadJSONDataInternal()
{
	updateMediaLoads();
}
