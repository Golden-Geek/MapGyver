/*
  ==============================================================================

    MediaSolidColor.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaSolidColor :
    public Media
{
public:
    MediaSolidColor(var params = var());
    ~MediaSolidColor();

    ColorParameter* color;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    
    String getTypeString() const override { return "SolidColor"; }
    static MediaSolidColor* create(var params) { return new MediaSolidColor(); };

    //virtual MediaUI* createUI() {return new MediaSolidColor(); };
};