/*
  ==============================================================================

	WebcamDevice.cpp
	Created: 20 Dec 2016 1:17:56pm
	Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

WebcamDevice::WebcamDevice(String& n, Type t) :
	name(n),
	type(t)
{}


WebcamInputDevice::WebcamInputDevice(String& name) :
	WebcamDevice(name, Webcam_IN)
{
}

WebcamInputDevice::~WebcamInputDevice()
{
}



void WebcamInputDevice::addWebcamInputListener(WebcamInputListener* newListener)
{
	inputListeners.add(newListener);
	if (inputListeners.size() == 1)
	{
		MessageManager::callAsync([this]()
		{
			StringArray names = CameraDevice::getAvailableDevices();
			int index = names.indexOf(name);
			device = CameraDevice::openDevice(index, 128, 64, 1920, 1080, true);
			if (device != nullptr) {
				device->addListener(this);
				shouldProcess = true;
			}
			else {
				LOGERROR("Error opening device");
			}
		});
	}
}

void WebcamInputDevice::removeWebcamInputListener(WebcamInputListener* listener) 
{
	inputListeners.remove(listener);
	if (inputListeners.size() == 0 && device != nullptr)
	{
		device->removeListener(this);
		delete(device);
		shouldProcess = false;
		//LOG("close connexion here");
	}
}

void WebcamInputDevice::imageReceived(const Image& image) {
	inputListeners.call(&WebcamInputDevice::WebcamInputListener::WebcamImageReceived, image);
}