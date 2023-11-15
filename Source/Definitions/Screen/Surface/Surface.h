/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class SurfaceOutput;

// #include "../Command/CommandSelectionManager.h"
class CommandSelectionManager;

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

    void onContainerParameterChangedInternal(Parameter* p);

    String getTypeString() const override { return objectType; }
    static Surface* create(var params) { return new Surface(params); }
};


