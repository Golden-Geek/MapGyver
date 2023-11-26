/*
  ==============================================================================

    WebcamDeviceParameter.h
    Created: 20 Dec 2016 3:05:54pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WebcamDeviceParameterUI;

class WebcamDeviceParameter :
	public Parameter,
	public WebcamManager::WebcamManagerListener
{
public:
	WebcamDeviceParameter(const String &name);
	~WebcamDeviceParameter();

	WebcamInputDevice * inputDevice;
	
	String ghostDeviceIn;
	String ghostDeviceOut;
	String ghostDeviceNameIn;
	String ghostDeviceNameOut;

	void setInputDevice(WebcamInputDevice * i);

	// Inherited via Listener
	virtual void WebcamDeviceInAdded(WebcamInputDevice *) override;
	virtual void WebcamDeviceInRemoved(WebcamInputDevice *) override;

	WebcamDeviceParameterUI* createWebcamParameterUI(Array<WebcamDeviceParameter*> parameters = {});
	ControllableUI * createDefaultUI(Array<Controllable*> controllables = {}) override;

	void loadJSONDataInternal(var data) override;

	String getTypeString() const override { return "WebcamDevice"; }

};