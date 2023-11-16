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

    factory.defs.add(Factory<Media>::Definition::createDef("", "SolidColor", &MediaSolidColor::create));
    factory.defs.add(Factory<Media>::Definition::createDef("", "Image", &MediaImage::create));
    factory.defs.add(Factory<Media>::Definition::createDef("", "VideoFile", &MediaVideo::create));
    factory.defs.add(Factory<Media>::Definition::createDef("", "NDI", &MediaNDI::create));
    factory.defs.add(Factory<Media>::Definition::createDef("", "Composition", &MediaComposition::create));

    itemDataType = "Media";
    selectItemWhenCreated = true;
    //autoReorderOnAdd = true;
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

