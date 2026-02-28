/*
  ==============================================================================

	fListMedia.cpp
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListMedia::MediaListMedia(var params) :
	Media(getTypeString(), params, true),
	currentMediaItem(nullptr)
{
	saveAndLoadRecursiveData = true;

	numLayers = mediaParams.addIntParameter("Num layers", "Number of layers in the list", 1, 1);
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
			item->load(transitionTime, currentMediaItem);
		else
			item->unload(transitionTime);
	}

	currentMediaItem = currentItem;
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
	else if (c == numLayers)
	{
		GlContextHolder::getInstance()->callOnGLThread([this]()
			{
				updateNumLayers();
			});
	}
}


void MediaListMedia::preRenderGLInternal()
{
	for (auto& i : listManager.items)
	{
		i->process();
	}
}


void MediaListMedia::renderGLInternal()
{
	


	for (int i = 0; i < numLayers->intValue(); i++)
	{
		OpenGLFrameBuffer* fbo = i == 0 ? &frameBuffer : extraFrameBuffers[i - 1];
		if (fbo->getWidth() != frameBuffer.getWidth() || fbo->getHeight() != frameBuffer.getHeight()) initExtraFrameBuffer(*fbo);
		fbo->makeCurrentAndClear();
		Init2DViewport(fbo->getWidth(), fbo->getHeight());
		glEnable(GL_BLEND);
		glColor4f(1, 0, 1, 1);
		Draw2DRect(0, 0, fbo->getWidth(), fbo->getHeight());
		renderLayer(i);
		fbo->releaseAsRenderingTarget();
	}
}

void MediaListMedia::renderLayer(int index)
{
	GenericScopedLock lock(listManager.items.getLock());


	for (auto& item : listManager.items)
	{
		GLuint textureID = item != nullptr ? item->getTextureIDAt(index) : 0;
		Media* media = item != nullptr ? item->getMediaAt(index) : nullptr;
		if (textureID == GLuint())
		{
			continue;
		}

		float w = item->weight->floatValue();

		if (w == 0.f) continue;

		ShaderMedia* shaderMedia = item->getShaderMediaAt(index);
		GLuint textureToUse = textureID;
		Media* mediaToUse = media;
		bool useShader = item->isLoading() && shaderMedia != nullptr && shaderMedia->enabled->boolValue() && shaderMedia->shaderLoaded->boolValue();
		if (shaderMedia != nullptr && useShader)
		{
			//later implement shader transition
			textureToUse = shaderMedia->getTextureID();
			mediaToUse = shaderMedia;
			w = 1.f;
		}


		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, textureToUse);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Rectangle<int> mediaRect = mediaToUse->getMediaRect(Rectangle<int>(0, 0, width->intValue(), height->intValue()));
		glColor4f(1, 1, 1, w);

		Draw2DTexRect(mediaRect.getX(), mediaRect.getY(), mediaRect.getWidth(), mediaRect.getHeight());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void MediaListMedia::updateNumLayers()
{

	int newNumLayers = numLayers->intValue();
	for (auto& i : listManager.items)
	{
		i->setNumLayers(numLayers->intValue());
	}

	while (extraFrameBuffers.size() < newNumLayers - 1)
	{
		extraFrameBuffers.add(std::make_unique<OpenGLFrameBuffer>());
		addFrameBuffer("Layer " + String(extraFrameBuffers.size() + 1), extraFrameBuffers.getLast());
	}

	while (extraFrameBuffers.size() > newNumLayers - 1)
	{
		removeFrameBuffer("Layer " + String(extraFrameBuffers.size()));
		extraFrameBuffers.removeLast();
	}
}


void MediaListMedia::itemAdded(MediaListItem* item)
{
	item->setNumLayers(numLayers->intValue());
	item->addAsyncMediaListItemListener(this);
}

void MediaListMedia::itemsAdded(juce::Array<MediaListItem*> items)
{
	for (auto& item : items)
	{
		item->setNumLayers(numLayers->intValue());
		item->addAsyncMediaListItemListener(this);
	}
}

void MediaListMedia::itemRemoved(MediaListItem* item)
{
	if (item == currentMediaItem) currentMediaItem = nullptr;
	item->removeAsyncMediaListItemListener(this);
}

void MediaListMedia::itemsRemoved(juce::Array<MediaListItem*> items)
{
	for (auto& item : items)
	{
		if (item == currentMediaItem) currentMediaItem = nullptr;
		item->removeAsyncMediaListItemListener(this);
	}
}

void MediaListMedia::initFrameBuffer()
{
	Media::initFrameBuffer();
	for (auto& fb : extraFrameBuffers)
	{
		initExtraFrameBuffer(*fb);
	}
}

void MediaListMedia::initExtraFrameBuffer(OpenGLFrameBuffer& fb)
{
	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (fb.isValid()) fb.release();
	fb.initialise(GlContextHolder::getInstance()->context, size.x, size.y);
	shouldRedraw = true;
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
