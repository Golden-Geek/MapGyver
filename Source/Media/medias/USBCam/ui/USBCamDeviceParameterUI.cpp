/*
  ==============================================================================

    USBCamDeviceParameterUI.cpp
    Created: 20 Dec 2016 3:06:05pm
    Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

USBCamDeviceParameterUI::USBCamDeviceParameterUI(Array<USBCamDeviceParameter *> _USBCamParams) :
	ParameterUI(Inspectable::getArrayAs<USBCamDeviceParameter, Parameter>(_USBCamParams)),
	USBCamParams(_USBCamParams),
	USBCamParam(_USBCamParams[0])
{
	addAndMakeVisible(&chooser);
	chooser.addUSBCamChooserListener(this);
	chooser.setSelectedInputDevice(USBCamParam->inputDevice);
	chooser.setGhostValue(USBCamParam->ghostDeviceIn);

	setSize(100, 20);
}

USBCamDeviceParameterUI::~USBCamDeviceParameterUI()
{
}

void USBCamDeviceParameterUI::resized()
{
	chooser.setBounds(getLocalBounds());
}

void USBCamDeviceParameterUI::valueChanged(const var & /*value*/)
{
	chooser.setSelectedInputDevice(USBCamParam->inputDevice);
	chooser.setGhostValue(USBCamParam->ghostDeviceNameIn);
}

void USBCamDeviceParameterUI::USBCamDeviceInSelected(USBCamInputDevice * d)
{
	USBCamParam->setInputDevice(d);
}

