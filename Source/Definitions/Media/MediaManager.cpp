#include "Media.h"
#include "MediaManager.h"

#include "Definitions/Media/medias/MediaSolidColor/MediaSolidColor.h"
#include "Definitions/Media/medias/MediaImage/MediaImage.h"
#include "Definitions/Media/medias/MediaVideo/MediaVideo.h"
#include "Definitions/Media/medias/MediaNDI/MediaNDI.h"
#include "Definitions/Media/medias/MediaComposition/MediaComposition.h"

/*
  ==============================================================================

    ObjectManager.cpp
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

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

