/*
  ==============================================================================

    NDIDeviceParameterUI.h
    Created: 20 Dec 2016 3:06:05pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "NDIDeviceChooser.h"
#include "../NDIDeviceParameter.h"

class NDIDeviceParameterUI :
	public ParameterUI,
	public NDIDeviceChooser::ChooserListener
{
public:
	NDIDeviceParameterUI(Array<NDIDeviceParameter *> NDIParam);
	~NDIDeviceParameterUI();
	
	Array<NDIDeviceParameter *> NDIParams;
	NDIDeviceParameter* NDIParam;
	NDIDeviceChooser chooser;
	
	void resized() override;

	void valueChanged(const var &value) override;

	void NDIDeviceInSelected(NDIInputDevice * d) override;
};