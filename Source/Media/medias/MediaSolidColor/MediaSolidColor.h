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

    void setColor(Colour c);
    
    DECLARE_TYPE("Solid Color")
};