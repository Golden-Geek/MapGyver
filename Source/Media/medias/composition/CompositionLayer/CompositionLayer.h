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

    void onContainerParameterChangedInternal(Parameter* p);

    bool isUsingMedia(Media* m) override;

    String getTypeString() const override { return objectType; }
    static CompositionLayer* create(var params) { return new CompositionLayer(params); }
};


