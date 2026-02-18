/*
  ==============================================================================

	MediaInteractionContainer.h
	Created: 18 Feb 2026 12:06:51pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaInteractionContainer :
	public ControllableContainer
{

public:
	MediaInteractionContainer();
	~MediaInteractionContainer();
	

	Point2DParameter* mousePosition;
	BoolParameter* leftButtonPressed;
	BoolParameter* rightButtonPressed;
	BoolParameter* middleButtonPressed;

	StringParameter* combinationToSend;
	BoolParameter* keyPressed;
};