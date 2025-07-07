/*
  ==============================================================================

    CVScreenManagerUI.h
    Created: 22 Feb 2018 3:42:48pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


class ScreenManagerUI :
	public ManagerShapeShifterUI<Manager<Screen>, Screen, ItemUI<Screen>>
{
public:
	ScreenManagerUI(const String &contentName);
	~ScreenManagerUI();

	static ScreenManagerUI * create(const String &name) { return new ScreenManagerUI(name); }
};