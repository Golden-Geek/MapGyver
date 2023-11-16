/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

juce_ImplementSingleton(MediaManager);

MediaManager::MediaManager() :
    BaseManager("Media")
{
    managerFactory = &factory;

    factory.defs.add(Factory<Media>::Definition::createDef<MediaSolidColor>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<MediaImage>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<MediaVideo>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<MediaNDI>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<MediaComposition>(""));

    itemDataType = "Media";
    selectItemWhenCreated = true;
}

MediaManager::~MediaManager()
{
    // stopThread(1000);
}


void MediaManager::addItemInternal(Media* o, var data)
{
    reorderItems();
}

void MediaManager::removeItemInternal(Media* o)
{
}

void MediaManager::onContainerParameterChanged(Parameter* p)
{
}

