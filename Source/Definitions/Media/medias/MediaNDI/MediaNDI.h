/*
  ==============================================================================

    MediaNDI.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once

//#include "../../Common/CommonIncludes.h"
#include "Definitions/Media/Media.h"
#include "Common/NDI/NDIDeviceParameter.h"

class FixturePatch;

class MediaNDI :
    public Media
{
public:
    MediaNDI(var params = var());
    ~MediaNDI();

    NDIDeviceParameter* ndiParam;
    ColorParameter* color;

    void clearItem() override;
    void onContainerParameterChanged(Parameter* p) override;
    
    String getTypeString() const override { return "NDI"; }
    static MediaNDI* create(var params) { return new MediaNDI(); };

    //virtual MediaUI* createUI() {return new MediaNDI(); };
};