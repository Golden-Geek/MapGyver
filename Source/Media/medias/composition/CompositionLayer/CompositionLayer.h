/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once


class CompositionLayer :
    public BaseItem,
    public MediaTarget
{
public:
    CompositionLayer(var params = var());
    virtual ~CompositionLayer();

    String objectType;
    var objectData;

    TargetParameter* media;
    Point2DParameter* position;
    Point2DParameter* size;
    FloatParameter* alpha;
    FloatParameter* rotation;

    enum blendPreset {STANDARD, ADDITION, MULTIPLICATION, SCREEN, DARKEN, PREMULTALPHA, LIGHTEN, INVERT, COLORADD, COLORSCREEN, BLUR, INVERTCOLOR, SUBSTRACT, COLORDIFF, INVERTMULT, CUSTOM
    };
    enum blendOption {ZERO, ONE, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, DST_ALPHA, ONE_MINUS_DST_ALPHA, SRC_COLOR, ONE_MINUS_SRC_COLOR, DST_COLOR, ONE_MINUS_DST_COLOR };

    EnumParameter* blendFunction;
    EnumParameter* blendFunctionSourceFactor;
    EnumParameter* blendFunctionDestinationFactor;

    void onContainerParameterChangedInternal(Parameter* p);

    bool isUsingMedia(Media* m) override;

    String getTypeString() const override { return objectType; }
    static CompositionLayer* create(var params) { return new CompositionLayer(params); }
};


