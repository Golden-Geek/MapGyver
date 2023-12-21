/*
  ==============================================================================

    MediaLayer.cpp
    Created: 21 Dec 2023 10:39:51am
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaLayer.h"

MediaLayer::MediaLayer(Sequence* s, var params) :
	SequenceLayer(s, "Media"),
	blockManager(this)
{
	saveAndLoadRecursiveData = true;

	addChildControllableContainer(&blockManager);
}

MediaLayer::~MediaLayer()
{
}

void MediaLayer::renderGL()
{
	float time = sequence->currentTime->floatValue();
	Array<LayerBlock*> blocks = blockManager.getBlocksAtTime(time, false);

	for (auto& b : blocks)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			if (clip->media == nullptr) continue;
			//clip->media->frameBuffer
		}
	}
}


SequenceLayerTimeline* MediaLayer::getTimelineUI()
{
	return new MediaLayerTimeline(this);
}
