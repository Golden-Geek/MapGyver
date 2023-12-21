/*
  ==============================================================================

    MediaLayerTimeline.h
    Created: 21 Dec 2023 11:01:14am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaLayerTimeline :
    public SequenceLayerTimeline
{
public:
    MediaLayerTimeline(MediaLayer* l);
    ~MediaLayerTimeline();

    MediaLayer* mediaLayer;
    MediaClipManagerUI clipManagerUI;

    void resized() override;
    void updateContent() override;
    virtual void updateMiniModeUI() override;

    virtual void addSelectableComponentsAndInspectables(Array<Component*>& selectables, Array<Inspectable*>& inspectables) override;
};
