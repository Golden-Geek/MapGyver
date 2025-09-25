/*
  ==============================================================================

    GridMedia.cpp
    Created: 11 Feb 2025 3:02:19pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "GridMedia.h"

GridMedia::GridMedia(var params) :
    Media(getTypeString(), params),
	clipManager(this)
{
    //addChildControllableContainer(&layerGroupManager);
    addChildControllableContainer(&columnManager);
	addChildControllableContainer(&clipManager);

    if (!Engine::mainEngine->isLoadingFile) createDefaultSetup();
}

GridMedia::~GridMedia()
{
}

void GridMedia::createDefaultSetup()
{
 //   auto layerGroup = layerGroupManager.addItem();
	//auto layer = layerGroup->layerManager.addItem();
 //   auto column = columnManager.addItem();

 //   GridClip* clip = clipManager.factory.create(ColorMedia::getTypeStringStatic());
 //   clip->layerTarget->setValueFromTarget(layer);
 //   clip->columnTarget->setValueFromTarget(column);
 //   
 //   clipManager.addItem(clip);
}
