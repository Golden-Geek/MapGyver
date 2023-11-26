/*
  ==============================================================================

    WebcamDeviceParameterUI.cpp
    Created: 20 Dec 2016 3:06:05pm
    Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

WebcamDeviceParameterUI::WebcamDeviceParameterUI(Array<WebcamDeviceParameter *> _WebcamParams) :
	ParameterUI(Inspectable::getArrayAs<WebcamDeviceParameter, Parameter>(_WebcamParams)),
	WebcamParams(_WebcamParams),
	WebcamParam(_WebcamParams[0])
{
	addAndMakeVisible(&chooser);
	chooser.addWebcamChooserListener(this);
	chooser.setSelectedInputDevice(WebcamParam->inputDevice);
	chooser.setGhostValue(WebcamParam->ghostDeviceIn);

	setSize(100, 20);
}

WebcamDeviceParameterUI::~WebcamDeviceParameterUI()
{
}

void WebcamDeviceParameterUI::resized()
{
	chooser.setBounds(getLocalBounds());
}

void WebcamDeviceParameterUI::valueChanged(const var & /*value*/)
{
	chooser.setSelectedInputDevice(WebcamParam->inputDevice);
	chooser.setGhostValue(WebcamParam->ghostDeviceNameIn);
}

void WebcamDeviceParameterUI::WebcamDeviceInSelected(WebcamInputDevice * d)
{
	WebcamParam->setInputDevice(d);
}

