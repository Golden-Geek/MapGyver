/*
  ==============================================================================

	MediaListCustomOrderManager.cpp
	Created: 2025
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListCustomOrderManager::MediaListCustomOrderManager() :
	Manager("Custom Order")
{
	itemDataType = "MediaListCustomOrderList";
	selectItemWhenCreated = false;
}

MediaListCustomOrderManager::~MediaListCustomOrderManager()
{
}
