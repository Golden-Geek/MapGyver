/*
  ==============================================================================

	MediaClipManagerUI.cpp
	Created: 21 Dec 2023 11:01:50am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaClipManagerUI::MediaClipManagerUI(MediaLayerTimeline* timeline) :
	LayerBlockManagerUI(timeline, &timeline->mediaLayer->blockManager),
	mediaTimeline(timeline)
{
	addExistingItems();
}

MediaClipManagerUI::~MediaClipManagerUI()
{
}


LayerBlockUI* MediaClipManagerUI::createUIForItem(LayerBlock* b)
{
	return new MediaClipUI((MediaClip*)b);
}

