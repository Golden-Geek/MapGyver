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

    void setTargetAutomation(ParameterAutomation* a);

    void paint(Graphics& g) override;
    void paintOverChildren(Graphics& g) override;

    void resizedBlockInternal() override;

    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void controllableFeedbackUpdateInternal(Controllable*) override;
    void newMessage(const MediaClip::MediaClipEvent& e) override;

};
