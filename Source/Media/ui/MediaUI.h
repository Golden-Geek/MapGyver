/*
  ==============================================================================

    MediaUI.h
    Created: 22 Nov 2023 3:38:52pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaUI :
    public BaseItemUI<Media>,
    public Media::AsyncListener
{
public:
    MediaUI(Media* item);
    virtual ~MediaUI();

    Image icon;
    Rectangle<int> iconBounds;
    Rectangle<int> infoBounds;

    void paint(Graphics& g) override;
    void resizedHeader(Rectangle<int> &r) override;

    void updateUI();
    void newMessage(const Media::MediaEvent& e) override;
    void controllableFeedbackUpdateInternal(Controllable* c) override;
};

