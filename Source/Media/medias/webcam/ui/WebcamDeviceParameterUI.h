/*
  ==============================================================================

    WebcamDeviceParameterUI.h
    Created: 20 Dec 2016 3:06:05pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WebcamDeviceParameterUI :
	public ParameterUI,
	public WebcamDeviceChooser::ChooserListener
{
public:
	WebcamDeviceParameterUI(Array<WebcamDeviceParameter *> WebcamParam);
	~WebcamDeviceParameterUI();
	
	Array<WebcamDeviceParameter *> WebcamParams;
	WebcamDeviceParameter* WebcamParam;
	WebcamDeviceChooser chooser;
	
	void resized() override;

	void valueChanged(const var &value) override;

	void WebcamDeviceInSelected(WebcamInputDevice * d) override;
};