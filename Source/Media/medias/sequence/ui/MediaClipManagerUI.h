/*
  ==============================================================================

    MediaClipManagerUI.h
    Created: 21 Dec 2023 11:01:50am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaLayerTimeline;

class MediaClipManagerUI :
    public LayerBlockManagerUI
{
public:
    MediaClipManagerUI(MediaLayerTimeline* timeline);
    ~MediaClipManagerUI();

    MediaLayerTimeline* mediaTimeline;

    LayerBlockUI* createUIForItem(LayerBlock* b) override;
};