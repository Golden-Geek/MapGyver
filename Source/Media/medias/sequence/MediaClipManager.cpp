/*
  ==============================================================================

	MediaClipManager.cpp
	Created: 21 Dec 2023 11:00:38am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaClipManager.h"

MediaClipManager::MediaClipManager(MediaLayer* layer) :
	LayerBlockManager(layer, "Blocks"),
	mediaLayer(layer)
{
	managerFactory = MediaClipFactory::getInstance();
	comparator.compareFunc = &MediaClipManager::compareTimeAndType;
}

MediaClipManager::~MediaClipManager()
{
}

LayerBlock* MediaClipManager::createItem()
{
	return new ReferenceMediaClip();
}

void MediaClipManager::addItemInternal(LayerBlock* block, var data)
{
	LayerBlockManager::addItemInternal(block, data);
	MediaClip* clip = dynamic_cast<MediaClip*>(block);
	clip->addMediaClipListener(this);

	if (auto t = dynamic_cast<ClipTransition*>(clip))
	{
		if (!isCurrentlyLoadingData)
		{
			reorderItems();
			if (t->inClip == nullptr || t->outClip == nullptr)
			{
				MediaClip* inMedia = dynamic_cast<MediaClip*>(items[items.indexOf(t) - 1]);
				MediaClip* outMedia = dynamic_cast<MediaClip*>(items[items.indexOf(t) + 1]);
				t->setInOutClips(inMedia, outMedia);
			}

			computeFadesForBlock(t->inClip, false);
			computeFadesForBlock(t->outClip, false);
		}
	}
}

void MediaClipManager::addItemsInternal(Array<LayerBlock*> blocks, var data)
{
	LayerBlockManager::addItemsInternal(blocks, data);
	for (auto& b : blocks)
	{
		MediaClip* clip = dynamic_cast<MediaClip*>(b);
		clip->addMediaClipListener(this);
	}
}

void MediaClipManager::removeItemInternal(LayerBlock* block)
{
	LayerBlockManager::removeItemInternal(block);
	MediaClip* clip = dynamic_cast<MediaClip*>(block);
	clip->removeMediaClipListener(this);
}

void MediaClipManager::removeItemsInternal(Array<LayerBlock*> blocks)
{
	LayerBlockManager::removeItemsInternal(blocks);
	for (auto& b : blocks)
	{
		MediaClip* clip = dynamic_cast<MediaClip*>(b);
		clip->removeMediaClipListener(this);
	}
}

void MediaClipManager::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	MediaClip* b = c->getParentAs<MediaClip >();
	if (b != nullptr)
	{
		if (c == b->time || c == b->coreLength || c == b->loopLength)
		{
			if (!blocksCanOverlap) return;
			if (dynamic_cast<ClipTransition*>(b) != nullptr) return;
			computeFadesForBlock(b, true);
		}
	}

	LayerBlockManager::onControllableFeedbackUpdate(cc, c);
}


void MediaClipManager::mediaClipFadesChanged(MediaClip* block)
{
	computeFadesForBlock(block, false);
}

void MediaClipManager::computeFadesForBlock(MediaClip* block, bool propagate)
{
	if (block == nullptr) return;
	if (dynamic_cast<ClipTransition*>(block) != nullptr) return;


	int bIndex = items.indexOf(block);

	MediaClip* prevBlock = bIndex > 0 ? (MediaClip*)items[bIndex - 1] : nullptr;
	MediaClip* nextBlock = bIndex < items.size() - 1 ? (MediaClip*)items[bIndex + 1] : nullptr;

	if (prevBlock != nullptr && prevBlock->time->floatValue() > block->time->floatValue())
	{
		if (dynamic_cast<ClipTransition*>(prevBlock) == nullptr)
		{
			reorderItems();
			computeFadesForBlock(block, propagate);
			return;
		}

	}

	if (nextBlock != nullptr && nextBlock->time->floatValue() < block->time->floatValue())
	{
		if (dynamic_cast<ClipTransition*>(nextBlock) == nullptr)
		{
			reorderItems();
			computeFadesForBlock(block, propagate);
			return;
		}
	}



	if (block->inTransition != nullptr) block->fadeIn->setValue(0);
	else if (!block->fadeIn->enabled)
	{
		float fadeIn = prevBlock == nullptr ? 0 : jmax(prevBlock->getEndTime() - block->time->floatValue(), 0.f);
		block->fadeIn->setValue(fadeIn);
	}

	if (block->outTransition != nullptr) block->fadeOut->setValue(0);
	else if (!block->fadeOut->enabled)
	{
		float fadeOut = nextBlock == nullptr ? 0 : jmax(block->getEndTime() - nextBlock->time->floatValue(), 0.f);
		block->fadeOut->setValue(fadeOut);
	}

	if (propagate)
	{
		if (prevBlock != nullptr) computeFadesForBlock(prevBlock, false);
		if (nextBlock != nullptr) computeFadesForBlock(nextBlock, false);
	}
}

int MediaClipManager::compareTimeAndType(LayerBlock* a, LayerBlock* b)
{
	if (a->time->floatValue() < b->time->floatValue()) return -1;
	else if (a->time->floatValue() > b->time->floatValue()) return 1;

	if (auto t = dynamic_cast<ClipTransition*>(a))
	{
		if (t->inClip == (MediaClip*)b) return 1;
		if (t->outClip == (MediaClip*)b) return -1;
	}
	if (auto t = dynamic_cast<ClipTransition*>(b))
	{
		if (t->inClip == (MediaClip*)a) return -1;
		if (t->outClip == (MediaClip*)a) return 1;
	}

	return 0;
}


juce_ImplementSingleton(MediaClipFactory)

MediaClipFactory::MediaClipFactory()
{
	defs.add(Definition::createDef<ReferenceMediaClip>(""));
	for (auto& md : MediaManager::getInstance()->factory.defs)
	{
		defs.add(Definition::createDef("Owned", md->type, &OwnedMediaClip::create)->addParam("mediaType", md->type));
	}
	defs.add(Definition::createDef<ClipTransition>(""));
}

