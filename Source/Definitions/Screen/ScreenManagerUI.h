/*
  ==============================================================================

    CVScreenManagerUI.h
    Created: 22 Feb 2018 3:42:48pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Screen.h"

class ScreenManagerUI :
	public BaseManagerShapeShifterUI<BaseManager<Screen>, Screen, BaseItemUI<Screen>>
{
public:
	ScreenManagerUI(const String &contentName);
	~ScreenManagerUI();

	static ScreenManagerUI * create(const String &name) { return new ScreenManagerUI(name); }
};