/*
  ==============================================================================

	MGSettings.h
	MapGyver-specific application settings

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class MGSettings :
	public ControllableContainer
{
public:
	juce_DeclareSingleton(MGSettings, true);

	MGSettings();
	~MGSettings();

	// Media settings
	ControllableContainer mediaCC;
	enum VideoEngine { ENGINE_MPV, ENGINE_VLC };
	EnumParameter* videoPlaybackEngine;
};
