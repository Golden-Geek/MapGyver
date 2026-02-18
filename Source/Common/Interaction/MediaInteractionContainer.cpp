/*
  ==============================================================================

	MediaInteractionContainer.cpp
	Created: 18 Feb 2026 12:06:51pm
	Author:  bkupe

  ==============================================================================
*/

#include "Common/CommonIncludes.h"

MediaInteractionContainer::MediaInteractionContainer() :
	ControllableContainer("Interaction")
{
	mousePosition = addPoint2DParameter("Mouse Position", "Position of the mouse to send, 0-1 relative to the media size");
	mousePosition->setBounds(0, 0, 1, 1);
	leftButtonPressed = addBoolParameter("Left Button", "Whether the left mouse button is pressed", false);
	rightButtonPressed = addBoolParameter("Right Button", "Whether the right mouse button is pressed", false);
	middleButtonPressed = addBoolParameter("Middle Button", "Whether the middle mouse button is pressed", false);

	combinationToSend = addStringParameter("Combination to Send", "Combination to send when keyPressed is triggered, in the format 'ctrl+shift+a' or 'left' for mouse buttons", "");
	keyPressed = addBoolParameter("Key Pressed", "Whether the key is pressed", false);
}

MediaInteractionContainer::~MediaInteractionContainer()
{
}
