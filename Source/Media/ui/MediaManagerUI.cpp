/*
  ==============================================================================

	ChannelFamilyManagerUI.cpp
	Created: 4 Nov 2021 12:32:12am
	Author:  No

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaManagerUI::MediaManagerUI(const String& contentName) :
	BaseManagerShapeShifterUI(contentName, MediaManager::getInstance())
{
	addItemText = "Add new Media";
	noItemText = "Medias are re-usable selections of SubFixtures.";
	// setShowAddButton(false);
	// setShowSearchBar(false);
	addExistingItems();

}

MediaManagerUI::~MediaManagerUI()
{
}

bool MediaManagerUI::isInterestedInDragSource(const SourceDetails& source)
{
	if (source.description.getProperty("type", "") == "OnlineContentItem") return true;
	return BaseManagerUI::isInterestedInDragSource(source);
}

void MediaManagerUI::itemDropped(const SourceDetails& source)
{
	BaseManagerUI::itemDropped(source);

	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		OnlineContentItem* item = (OnlineContentItem*)source.sourceComponent.get();
		if (item != nullptr)
		{
			if (Media* media = item->createMedia())
			{
				MediaManager::getInstance()->addItem(media);
			}
		}
	}
}



