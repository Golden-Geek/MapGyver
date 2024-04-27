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
	LayerBlockManager(layer, "Blocks")
{
	managerFactory = MediaClipFactory::getInstance();
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
			
			MediaClip* inMedia = dynamic_cast<MediaClip*>(items[items.indexOf(t) - 1]);
			MediaClip* outMedia = dynamic_cast<MediaClip*>(items[items.indexOf(t) + 1]);
			t->setInOutMedia(inMedia, outMedia);
			computeTransitionTimes();
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
			computeTransitionTimes();
			computeFadesForBlock(b, true);
		}
	}
}


void MediaClipManager::mediaClipFadesChanged(MediaClip* block)
{
	computeFadesForBlock(block, false);
}

void MediaClipManager::computeTransitionTimes()
{
	Array<ClipTransition*> transitions = getItemsWithType<ClipTransition>();

	for (auto& t : transitions)
	{
		if (t->inMedia != nullptr && t->outMedia != nullptr)
		{
			float inTime = t->inMedia->getEndTime();
			float outTime = t->outMedia->time->floatValue();
			float minTime = jmin(inTime, outTime);
			float maxTime = jmax(inTime, outTime);

			t->time->setValue(minTime);
			t->setCoreLength(maxTime - minTime);
			items.move(items.indexOf(t), items.indexOf(t->inMedia) + 1);
		}

	}
}

void MediaClipManager::computeFadesForBlock(MediaClip* block, bool propagate)
{
	int bIndex = items.indexOf(block);

	MediaClip* prevBlock = bIndex > 0 ? (MediaClip*)items[bIndex - 1] : nullptr;
	MediaClip* nextBlock = bIndex < items.size() - 1 ? (MediaClip*)items[bIndex + 1] : nullptr;



	if (prevBlock != nullptr && prevBlock->time->floatValue() > block->time->floatValue())
	{
		reorderItems();
		computeFadesForBlock(block, propagate);
		return;
	}

	if (nextBlock != nullptr && nextBlock->time->floatValue() < block->time->floatValue())
	{
		reorderItems();
		computeFadesForBlock(block, propagate);
		return;
	}

	bool prevBlockIsTransition = dynamic_cast<ClipTransition*>(prevBlock) != nullptr;
	bool nextBlockIsTransition = dynamic_cast<ClipTransition*>(nextBlock) != nullptr;

	if (!block->fadeIn->enabled && !prevBlockIsTransition)
	{
		float fadeIn = prevBlock == nullptr ? 0 : jmax(prevBlock->getEndTime() - block->time->floatValue(), 0.f);
		block->fadeIn->setValue(fadeIn);
	}

	if (!block->fadeOut->enabled && !prevBlockIsTransition)
	{
		float fadeOut = nextBlock == nullptr ? 0 : jmax(block->getEndTime() - nextBlock->time->floatValue(), 0.f);
		block->fadeOut->setValue(fadeOut);
	}

	if (propagate)
	{
		if (prevBlock != nullptr && !prevBlockIsTransition) computeFadesForBlock(prevBlock, false);
		if (nextBlock != nullptr && !nextBlockIsTransition) computeFadesForBlock(nextBlock, false);
	}
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
