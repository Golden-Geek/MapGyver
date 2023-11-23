/*
  ==============================================================================

    USBCamDeviceParameter.h
    Created: 20 Dec 2016 3:05:54pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class USBCamDeviceParameterUI;

class USBCamDeviceParameter :
	public Parameter,
	public USBCamManager::USBCamManagerListener
{
public:
	USBCamDeviceParameter(const String &name);
	~USBCamDeviceParameter();

	USBCamInputDevice * inputDevice;
	
	String ghostDeviceIn;
	String ghostDeviceOut;
	String ghostDeviceNameIn;
	String ghostDeviceNameOut;

	void setInputDevice(USBCamInputDevice * i);

	// Inherited via Listener
	virtual void USBCamDeviceInAdded(USBCamInputDevice *) override;
	virtual void USBCamDeviceInRemoved(USBCamInputDevice *) override;

	USBCamDeviceParameterUI* createUSBCamParameterUI(Array<USBCamDeviceParameter*> parameters = {});
	ControllableUI * createDefaultUI(Array<Controllable*> controllables = {}) override;

	void loadJSONDataInternal(var data) override;

	String getTypeString() const override { return "USBCamDevice"; }

};