/*
  ==============================================================================

    WebcamDeviceParameter.cpp
    Created: 20 Dec 2016 3:05:54pm
    Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

WebcamDeviceParameter::WebcamDeviceParameter(const String & name) :
	Parameter(CUSTOM, name, "Webcam Devices",var(), var(),var()),
	inputDevice(nullptr)
{
	WebcamManager::getInstance()->addWebcamManagerListener(this);
	value = "";
}

WebcamDeviceParameter::~WebcamDeviceParameter()
{
	if (WebcamManager::getInstanceWithoutCreating() != nullptr)
	{
		WebcamManager::getInstance()->removeWebcamManagerListener(this);
	}
}



void WebcamDeviceParameter::setInputDevice(WebcamInputDevice * i)
{
	var val = i != nullptr ? i->name : "";

	if (i != nullptr)
	{
		ghostDeviceIn = value;
		ghostDeviceNameIn = i->name;
	}

	inputDevice = i;

	setValue(val);
}

void WebcamDeviceParameter::WebcamDeviceInAdded(WebcamInputDevice * i)
{	
	//DBG("Device In added " << i->name << " / " << ghostDeviceIn);
	if (inputDevice == nullptr && i->name == ghostDeviceIn)
	{
		setInputDevice(nullptr);
		setInputDevice(i);
	}
}

void WebcamDeviceParameter::WebcamDeviceInRemoved(WebcamInputDevice * i)
{
	if (i == inputDevice)
	{
		if (i != nullptr)
		{
			ghostDeviceIn = i->name;
			ghostDeviceNameIn = i->name;
		}
		setInputDevice(nullptr);
	}
}

WebcamDeviceParameterUI * WebcamDeviceParameter::createWebcamParameterUI(Array<WebcamDeviceParameter *> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new WebcamDeviceParameterUI(parameters);
}

ControllableUI * WebcamDeviceParameter::createDefaultUI(Array<Controllable *> controllables)
{
	Array<WebcamDeviceParameter*> parameters = Inspectable::getArrayAs<Controllable, WebcamDeviceParameter>(controllables);
	if (parameters.size() == 0) parameters.add(this);
	return createWebcamParameterUI(parameters);
}

void WebcamDeviceParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	//setInputDevice(WebcamManager::getInstance()->getInputDeviceWithID(value[0]));

	if (inputDevice == nullptr) ghostDeviceIn = data.getProperty("value", var());	

}
