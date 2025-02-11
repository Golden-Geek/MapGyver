/*
  ==============================================================================

    MediaClipUI.h
    Created: 21 Dec 2023 10:40:59am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaClipFadeHandle :
    public Component
{
public:
    MediaClipFadeHandle();
    ~MediaClipFadeHandle() {}

    void paint(Graphics& g) override;
};


class MediaClipUI :
    public LayerBlockUI,
    public MediaClip::AsyncListener
{
public:
    MediaClipUI(MediaClip* b);
    ~MediaClipUI();

    MediaClip* mediaClip;

    float fadeValueAtMouseDown;
    MediaClipFadeHandle fadeInHandle;
    MediaClipFadeHandle fadeOutHandle;

    std::unique_ptr<Component> automationUI;

    Path clipPath;
	Path loopPath;
    Rectangle<float> usableCoreBounds;
	Rectangle<float> usableLoopBounds;

    void setTargetAutomation(ParameterAutomation* a);

    void paint(Graphics& g) override;
    void paintOverChildren(Graphics& g) override;

    void resizedBlockInternal() override;

    Path generatePath(bool isLoop = false);

    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void addContextMenuItems(PopupMenu &m) override;

    bool hitTest(int x, int y) override;

    bool isInterestedInDragSource(const SourceDetails& source) override;
    void itemDropped(const SourceDetails& source) override;


    void controllableFeedbackUpdateInternal(Controllable*) override;
    void newMessage(const MediaClip::MediaClipEvent& e) override;

};
