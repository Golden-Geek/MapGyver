/*
  ==============================================================================

	MediaListItemManager.cpp
	Created: 18 Feb 2026 8:29:38am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListItemManager::MediaListItemManager() :
	Manager("List Items")
{
	itemDataType = "MediaListItem";
	selectItemWhenCreated = false;
	managerFactory = MediaListItemFactory::getInstance();

	thumbSize = addFloatParameter("Thumb size", "Size of the thumbnail in pixels when displayed in the list", 100, 32, 500);
}

MediaListItemManager::~MediaListItemManager()
{
	// stopThread(1000);
}


juce_ImplementSingleton(MediaListItemFactory)

MediaListItemFactory::MediaListItemFactory()
{
	defs.add(Definition::createDef<ReferenceMediaListItem>(""));
	for (auto& md : MediaManager::getInstance()->factory.defs)
	{
		defs.add(Definition::createDef("Owned", md->type, &OwnedMediaListItem::create)->addParam("mediaType", md->type));
	}
}
