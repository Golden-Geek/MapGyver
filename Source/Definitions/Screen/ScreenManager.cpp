#include "Screen.h"
#include "ScreenManager.h"

/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

juce_ImplementSingleton(ScreenManager);

ScreenManager::ScreenManager() :
    BaseManager("Screen")
    {
    itemDataType = "Screen";
    selectItemWhenCreated = true;
}

ScreenManager::~ScreenManager()
{
    // stopThread(1000);
}


void ScreenManager::addItemInternal(Screen* o, var data)
{
}

void ScreenManager::removeItemInternal(Screen* o)
{
}

void ScreenManager::onContainerParameterChanged(Parameter* p)
{
}

