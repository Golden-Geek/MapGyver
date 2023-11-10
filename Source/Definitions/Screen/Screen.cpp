/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Screen.h"
#include "ScreenManager.h"
#include "ScreenOutput.h"

Screen::Screen(var params) :
	BaseItem(params.getProperty("name", "Screen")),
	objectType(params.getProperty("type", "Screen").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = false;

	itemDataType = "Screen";

	screenNumber = addIntParameter("Screen number", "Screen ID in your OS",0,0);
	isOn = addBoolParameter("Is on", "if checheckd, we'll display this screen",false);

	linkedScreenOutput = std::make_shared<ScreenOutput>();
	linkedScreenOutput->isOn = isOn;

}

Screen::~Screen()
{
}

void Screen::onContainerParameterChangedInternal(Parameter* p) {
	if (p == isOn) {
		if (isOn->boolValue()) {
			linkedScreenOutput->goLive(screenNumber->intValue());
		}
		else {
			linkedScreenOutput->stopLive();
		}
	}
	else if (p == screenNumber) {
		if (isOn->boolValue()) {
			linkedScreenOutput->stopLive();
			linkedScreenOutput->goLive(screenNumber->intValue());
		}
	}
}

