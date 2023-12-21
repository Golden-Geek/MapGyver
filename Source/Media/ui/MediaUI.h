/*
  ==============================================================================

    MediaUI.h
    Created: 22 Nov 2023 3:38:52pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaUI :
    public BaseItemUI<Media>
{
public:
    MediaUI(Media* item);
    virtual ~MediaUI();

    void mouseDoubleClick(const MouseEvent& e) override;
};

