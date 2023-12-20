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

    factory.defs.add(Factory<Media>::Definition::createDef<ColorMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<PictureMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<VideoMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<WebcamMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<NDIMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<SharedTextureMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<ShaderMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<CompositionMedia>(""));
    factory.defs.add(Factory<Media>::Definition::createDef<NodeMedia>(""));

    itemDataType = "Media";
    selectItemWhenCreated = true;

    ShaderCheckTimer::getInstance();
}

MediaManager::~MediaManager()
{
    // stopThread(1000);

    ShaderCheckTimer::deleteInstance();
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

