/*
  ==============================================================================

    MediaImage.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

#include "Definitions/Media/Media.h"

class FixturePatch;

class MediaImage :
    public Media
{
public:
    MediaImage(var params = var());
    ~MediaImage();

    FileParameter* filePath;

    void clearItem() override;
    void onContainerParameterChanged(Parameter* p) override;
    
    String getTypeString() const override { return "Image"; }
    static MediaImage* create(var params) { return new MediaImage(); };

    //virtual MediaUI* createUI() {return new MediaImage(); };
};