/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Surface :
    public BaseItem
{
public:
    Surface(var params = var());
    virtual ~Surface();

    String objectType;
    var objectData;

    Point2DParameter* topLeft;
    Point2DParameter* topRight;
    Point2DParameter* bottomLeft;
    Point2DParameter* bottomRight;

    FloatParameter* softEdgeTop;
    FloatParameter* softEdgeRight;
    FloatParameter* softEdgeBottom;
    FloatParameter* softEdgeLeft;

    FloatParameter* cropTop;
    FloatParameter* cropRight;
    FloatParameter* cropBottom;
    FloatParameter* cropLeft;

    TargetParameter* media;

    Path quadPath;

    void onContainerParameterChangedInternal(Parameter* p);

    void updatePath();

    bool isPointInside(Point<float> pos);

    String getTypeString() const override { return objectType; }
    static Surface* create(var params) { return new Surface(params); }
};


