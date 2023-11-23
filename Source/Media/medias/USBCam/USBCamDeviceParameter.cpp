/*
  ==============================================================================

    USBCamDeviceParameter.cpp
    Created: 20 Dec 2016 3:05:54pm
    Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

USBCamDeviceParameter::USBCamDeviceParameter(const String & name) :
	Parameter(CUSTOM, name, "USBCam Devices",var(), var(),var()),
	inputDevice(nullptr)
{
	USBCamManager::getInstance()->addUSBCamManagerListener(this);
	value = "";
}

USBCamDeviceParameter::~USBCamDeviceParameter()
{
	if (USBCamManager::getInstanceWithoutCreating() != nullptr)
	{
		USBCamManager::getInstance()->removeUSBCamManagerListener(this);
	}
}



void USBCamDeviceParameter::setInputDevice(USBCamInputDevice * i)
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

void USBCamDeviceParameter::USBCamDeviceInAdded(USBCamInputDevice * i)
{	
	//DBG("Device In added " << i->name << " / " << ghostDeviceIn);
	if (inputDevice == nullptr && i->name == ghostDeviceIn)
	{
		setInputDevice(i);
	}
}

void USBCamDeviceParameter::USBCamDeviceInRemoved(USBCamInputDevice * i)
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

USBCamDeviceParameterUI * USBCamDeviceParameter::createUSBCamParameterUI(Array<USBCamDeviceParameter *> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new USBCamDeviceParameterUI(parameters);
}

ControllableUI * USBCamDeviceParameter::createDefaultUI(Array<Controllable *> controllables)
{
	Array<USBCamDeviceParameter*> parameters = Inspectable::getArrayAs<Controllable, USBCamDeviceParameter>(controllables);
	if (parameters.size() == 0) parameters.add(this);
	return createUSBCamParameterUI(parameters);
}

void USBCamDeviceParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	//setInputDevice(USBCamManager::getInstance()->getInputDeviceWithID(value[0]));

	if (inputDevice == nullptr) ghostDeviceIn = data.getProperty("value", var());	

}
