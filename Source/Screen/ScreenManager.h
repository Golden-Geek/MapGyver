/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ScreenManager :
    public Manager<Screen>
{
public:
    juce_DeclareSingleton(ScreenManager, true);

    ScreenManager();
    ~ScreenManager();
     
    Screen* editingScreen;

    var getJSONData(bool includeNonOverriden = false) override;
    void loadJSONDataManagerInternal(var data) override;

};