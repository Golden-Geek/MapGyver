/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Media;

class Pin :
    public BaseItem{
public:
    Pin(var params = var());
    virtual ~Pin();

    String objectType;
    var objectData;

    Point2DParameter* position;
    Point2DParameter* mediaPos;
    FloatParameter* ponderation;

    String getTypeString() const override { return objectType; }
    static Pin* create(var params) { return new Pin(params); }
};


