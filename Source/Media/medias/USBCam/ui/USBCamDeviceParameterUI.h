/*
  ==============================================================================

    USBCamDeviceParameterUI.h
    Created: 20 Dec 2016 3:06:05pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class USBCamDeviceParameterUI :
	public ParameterUI,
	public USBCamDeviceChooser::ChooserListener
{
public:
	USBCamDeviceParameterUI(Array<USBCamDeviceParameter *> USBCamParam);
	~USBCamDeviceParameterUI();
	
	Array<USBCamDeviceParameter *> USBCamParams;
	USBCamDeviceParameter* USBCamParam;
	USBCamDeviceChooser chooser;
	
	void resized() override;

	void valueChanged(const var &value) override;

	void USBCamDeviceInSelected(USBCamInputDevice * d) override;
};