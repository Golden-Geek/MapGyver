/*
  ==============================================================================

    MediaLayerTimeline.cpp
    Created: 21 Dec 2023 11:01:14am
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaLayerTimeline::MediaLayerTimeline(MediaLayer* l) :
    SequenceLayerTimeline(l),
    mediaLayer(l),
    clipManagerUI(this)
{
    addAndMakeVisible(&clipManagerUI);
}

MediaLayerTimeline::~MediaLayerTimeline()
{
}

void MediaLayerTimeline::resized()
{
    clipManagerUI.setBounds(getLocalBounds());
}

void MediaLayerTimeline::updateContent()
{
    clipManagerUI.updateContent();
}

void MediaLayerTimeline::updateMiniModeUI()
{
    clipManagerUI.setMiniMode(item->miniMode->boolValue());
}

void MediaLayerTimeline::addSelectableComponentsAndInspectables(Array<Component*>& selectables, Array<Inspectable*>& inspectables)
{
    clipManagerUI.addSelectableComponentsAndInspectables(selectables, inspectables);
}
