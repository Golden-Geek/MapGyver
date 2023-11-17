/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

Screen::Screen(var params) :
	BaseItem(params.getProperty("name", "Screen")),
	objectType(params.getProperty("type", "Screen").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;

	itemDataType = "Screen";

	output.reset(new ScreenOutput(this));

	screenNumber = addIntParameter("Screen number", "Screen ID in your OS", 1, 0);
	//enabled->setDefaultValue(false);


	addChildControllableContainer(&surfaces);
}

Screen::~Screen()
{
}

void Screen::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled || p == screenNumber)
	{
		updateOutputLiveStatus();
	}
}

void Screen::updateOutputLiveStatus()
{
	if (isCurrentlyLoadingData) return;

	if (enabled->boolValue())
	{
		output->stopLive();
		output->goLive(screenNumber->intValue());
	}
	else
	{
		output->stopLive();
	}
}

void Screen::afterLoadJSONDataInternal()
{
	updateOutputLiveStatus();
}

