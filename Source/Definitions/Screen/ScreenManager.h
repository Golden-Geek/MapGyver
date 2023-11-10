/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "Screen.h"

class ScreenManager :
    public BaseManager<Screen>
{
public:
    juce_DeclareSingleton(ScreenManager, true);

    ScreenManager();
    ~ScreenManager();

    void addItemInternal(Screen* o, var data) override;
    void removeItemInternal(Screen* o) override;

    void onContainerParameterChanged(Parameter* p) override;

};