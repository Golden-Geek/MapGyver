/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Screen :
    public BaseItem
{
public:
    Screen(var params = var());
    virtual ~Screen();

    String objectType;
    var objectData;

    IntParameter* screenNumber;
    std::unique_ptr<ScreenOutput> output;

    SurfaceManager surfaces;

    void onContainerParameterChangedInternal(Parameter* p);

    void updateOutputLiveStatus();

    void afterLoadJSONDataInternal() override;

    String getTypeString() const override { return objectType; }
    static Screen* create(var params) { return new Screen(params); }
};


