
/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

SurfaceManager::SurfaceManager() :
    Manager("Surface")
    {
    itemDataType = "Surface";
    selectItemWhenCreated = false;
}

SurfaceManager::~SurfaceManager()
{
    // stopThread(1000);
}


void SurfaceManager::addItemInternal(Surface* o, var data)
{
    if (!isCurrentlyLoadingData)
    {
        setItemIndex(o, 0);
    }
}

void SurfaceManager::removeItemInternal(Surface* o)
{
}

void SurfaceManager::onContainerParameterChanged(Parameter* p)
{
}

