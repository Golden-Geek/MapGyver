/*
  ==============================================================================

    ChannelFamilyManagerUI.cpp
    Created: 4 Nov 2021 12:32:12am
    Author:  No

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

ScreenManagerUI::ScreenManagerUI(const String & contentName) :
	ManagerShapeShifterUI(contentName, ScreenManager::getInstance())
{
	addItemText = "Add new Screen";
	noItemText = "Draw here curves you wanna use often";
	// setShowAddButton(false);
	// setShowSearchBar(false);
	addExistingItems();
}

ScreenManagerUI::~ScreenManagerUI()
{
}
