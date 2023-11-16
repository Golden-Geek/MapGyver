/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

CompositionLayerManager::CompositionLayerManager() :
    BaseManager("CompositionLayer")
    {
    itemDataType = "CompositionLayer";
    selectItemWhenCreated = false;
}

CompositionLayerManager::~CompositionLayerManager()
{
    // stopThread(1000);
}


void CompositionLayerManager::addItemInternal(CompositionLayer* o, var data)
{
}

void CompositionLayerManager::removeItemInternal(CompositionLayer* o)
{
}

void CompositionLayerManager::onContainerParameterChanged(Parameter* p)
{
}

