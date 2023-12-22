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

    int dropClipX = -1;

    void paintOverChildren(Graphics& g) override;
    LayerBlockUI* createUIForItem(LayerBlock* b) override;

    bool isInterestedInDragSource(const SourceDetails& source) override;
    void itemDragEnter(const SourceDetails& source) override;
    void itemDragExit(const SourceDetails& source) override;
    void itemDragMove(const SourceDetails& source) override;
    void itemDropped(const SourceDetails& source) override;

    void showMenuAndAddItem(bool fromAddButton, Point<int> mouseDownPos) override;
    void mouseDoubleClick(const MouseEvent& e) override;
};