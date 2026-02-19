/*
  ==============================================================================

	fListMedia.cpp
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListMedia::MediaListMedia(var params) :
	Media(getTypeString(), params, true)
{
	saveAndLoadRecursiveData = true;

	index = mediaParams.addIntParameter("Index", "Index of the media to use", 1, 1);

	nextTrigger = mediaParams.addTrigger("Next", "Go to next media in the list");
	previousTrigger = mediaParams.addTrigger("Previous", "Go to previous media in the list");
	loop = mediaParams.addBoolParameter("Loop", "Whether to loop around when reaching the end of the list", false);
	addChildControllableContainer(&listManager);
	defaultTransitionTime = mediaParams.addFloatParameter("Default transition time", "Default transition time in seconds when not specified in the item", 1, 0);

	alwaysRedraw = true;


	listManager.addManagerListener(this);
}

MediaListMedia::~MediaListMedia()
{
}

void MediaListMedia::clearItem()
{
	Media::clearItem();
	listManager.removeManagerListener(this);
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
	float transitionTime = currentItem->transitionTime->enabled ? currentItem->transitionTime->floatValue() : defaultTransitionTime->floatValue();
	for (auto& item : listManager.items)
	{
		if (item == currentItem)
			item->load(transitionTime, currentMedia);
		else
			item->unload(transitionTime);
	}

	currentMedia = currentItem->media;
}

void MediaListMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (c == index)
	{
		updateMediaLoads();
	}
	else if (MediaListItem* item = dynamic_cast<MediaListItem*>(c->parentContainer.get()))
	{
		if (c == item->launch)
		{
			index->setValue(listManager.items.indexOf(item) + 1);
		}
	}
	else if (c == nextTrigger)
	{
		int nextIndex = index->intValue(); // -1 for 0-based index, but +1 to do the next item
		while (listManager.items.size() > nextIndex && !listManager.items[nextIndex]->enabled->boolValue())
		{
			nextIndex++;
		}

		if (nextIndex >= listManager.items.size())
		{
			if (loop->boolValue()) nextIndex = 0;
		}

		index->setValue(nextIndex + 1);
	}
	else if (c == previousTrigger)
	{

		int prevIndex = index->intValue() - 2; // -1 for 0-based index, but -1 to do the previous item
		while (prevIndex >= 0 && !listManager.items[prevIndex]->enabled->boolValue())
		{
			prevIndex--;
		}

		if (prevIndex < 0)
		{
			if (loop->boolValue()) prevIndex = listManager.items.size() - 1;
		}

		index->setValue(prevIndex + 1);
	}
}


void MediaListMedia::preRenderGLInternal()
{
	for (auto& i : listManager.items)
	{
		i->process();
		if (i->weight->floatValue() == 0.f) continue;
		i->media->renderOpenGLMedia();
		ShaderMedia* shaderMedia = i->shaderMedia.get();
		bool useShader = i->isLoading() && shaderMedia != nullptr && shaderMedia->enabled->boolValue() && shaderMedia->shaderLoaded->boolValue();
		if (useShader) i->shaderMedia->renderOpenGLMedia();

	}
}

void MediaListMedia::renderGLInternal()
{
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
		Media* mediaToUse = m;
		bool useShader = i->isLoading() && shaderMedia != nullptr && shaderMedia->enabled->boolValue() && shaderMedia->shaderLoaded->boolValue();
		if (shaderMedia != nullptr && useShader)
		{
			//later implement shader transition
			mediaToUse = shaderMedia;
			w = 1.f;
		}


		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, mediaToUse->getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Rectangle<int> mediaRect = mediaToUse->getMediaRect(Rectangle<int>(0, 0, width->intValue(), height->intValue()));
		glColor4f(1, 1, 1, w);

		Draw2DTexRect(mediaRect.getX(), mediaRect.getY(), mediaRect.getWidth(), mediaRect.getHeight());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}


void MediaListMedia::itemAdded(MediaListItem* item)
{
	item->addAsyncMediaListItemListener(this);
}

void MediaListMedia::itemsAdded(juce::Array<MediaListItem*> items)
{
	for (auto& item : items)
	{
		item->addAsyncMediaListItemListener(this);
	}
}

void MediaListMedia::itemRemoved(MediaListItem* item)
{
	item->removeAsyncMediaListItemListener(this);
}

void MediaListMedia::itemsRemoved(juce::Array<MediaListItem*> items)
{
	for (auto& item : items)
	{
		item->removeAsyncMediaListItemListener(this);
	}
}

void MediaListMedia::newMessage(const MediaListItem::MediaListItemEvent& e)
{
	if (e.type == MediaListItem::MediaListItemEvent::AUTO_NEXT)
	{
		nextTrigger->trigger();
	}
}

void MediaListMedia::afterLoadJSONDataInternal()
{
	updateMediaLoads();
}
