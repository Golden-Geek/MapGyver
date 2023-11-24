/*
  ==============================================================================

	USBCamDevice.cpp
	Created: 20 Dec 2016 1:17:56pm
	Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

USBCamDevice::USBCamDevice(String& n, Type t) :
	name(n),
	type(t)
{}


USBCamInputDevice::USBCamInputDevice(String& name) :
	USBCamDevice(name, USBCam_IN)
{
}

USBCamInputDevice::~USBCamInputDevice()
{
}



void USBCamInputDevice::addUSBCamInputListener(USBCamInputListener* newListener)
{
	inputListeners.add(newListener);
	if (inputListeners.size() == 1)
	{
		StringArray names = CameraDevice::getAvailableDevices();
		int index = names.indexOf(name);
		device = CameraDevice::openDevice(index, 128,64,1920,1080,true);
		if (device != nullptr) {
			device->addListener(this);
			shouldProcess = true;
		}
		else {
			LOGERROR("Error opening device");
		}
	}
}

void USBCamInputDevice::removeUSBCamInputListener(USBCamInputListener* listener) 
{
	inputListeners.remove(listener);
	if (inputListeners.size() == 0 && device != nullptr)
	{
		device->removeListener(this);
		delete(device);
		shouldProcess = false;
		LOG("close connexion here");
	}
}

void USBCamInputDevice::imageReceived(const Image& image) {
	inputListeners.call(&USBCamInputDevice::USBCamInputListener::usbCamImageReceived, image);
}