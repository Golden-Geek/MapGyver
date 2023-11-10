#include "Surface.h"
#include "SurfaceManager.h"

/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

SurfaceManager::SurfaceManager() :
    BaseManager("Surface")
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
}

void SurfaceManager::removeItemInternal(Surface* o)
{
}

void SurfaceManager::onContainerParameterChanged(Parameter* p)
{
}

