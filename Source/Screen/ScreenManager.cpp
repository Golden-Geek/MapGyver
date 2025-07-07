/*
  ==============================================================================

	ObjectManager.cpp
	Created: 26 Sep 2020 10:02:28am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

juce_ImplementSingleton(ScreenManager);

ScreenManager::ScreenManager() :
	Manager("Screen"),
	editingScreen(nullptr)
{
	itemDataType = "Screen";
	selectItemWhenCreated = true;
}

ScreenManager::~ScreenManager()
{
}


var ScreenManager::getJSONData(bool includeNonOverriden)
{
	var data = Manager::getJSONData(includeNonOverriden);
	if(editingScreen != nullptr) data.getDynamicObject()->setProperty("editingScreen", editingScreen->shortName);
	return data;
}

void ScreenManager::loadJSONDataManagerInternal(var data)
{
	Manager::loadJSONDataManagerInternal(data);

	if (data.hasProperty("editingScreen"))
	{
		String editingScreenName = data.getProperty("editingScreen", "Screen").toString();
		editingScreen = getItemWithName(editingScreenName);
	}
}

