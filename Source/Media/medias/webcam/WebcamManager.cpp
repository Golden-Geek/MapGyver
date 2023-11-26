/*
  ==============================================================================

	WebcamManager.cpp
	Created: 20 Dec 2016 12:33:33pm
	Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

juce_ImplementSingleton(WebcamManager)

WebcamManager::WebcamManager() :
    Thread("Webcam Manager")
{
	// WebcamRouterDefaultType = dynamic_cast<BKEngine *>(Engine::mainEngine)->defaultBehaviors.addEnumParameter("Webcam Router Ouput Type","Choose the default type when choosing a Webcam Module as Router output");
	// WebcamRouterDefaultType->addOption("Control Change", WebcamManager::CONTROL_CHANGE)->addOption("Note On", WebcamManager::NOTE_ON)->addOption("Note Off", WebcamManager::NOTE_OFF);

    startThread();
}

WebcamManager::~WebcamManager()
{
    stopThread(3000);
}

void WebcamManager::checkDevices()
{
	//INPUTS
	//LOG("searching for devices");
    StringArray names = CameraDevice::getAvailableDevices();

    for (int i = inputs.size()-1; i >= 0; i--) {
        String key = inputs[i]->name;
        bool isPresent = false;
        for (int j = 0; j < names.size() && !isPresent; j++) {
            isPresent = String(names[i]) == key;
        }
        if (!isPresent) {
            removeInputDevice(inputs[i]);
        }
    }

    if (names.size() > 0) {
        for (int i = 0; i < names.size(); i++) {
            WebcamInputDevice* input = getInputDeviceWithName(names[i]);
            if (input == nullptr) {
                input = addInputDeviceIfNotThere(names[i]);
            }
        }
    }
}

WebcamInputDevice* WebcamManager::addInputDeviceIfNotThere(String name)
{
	WebcamInputDevice* d = new WebcamInputDevice(name);
	inputs.add(d);

	NLOG("Webcam", "Device In Added : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&WebcamManagerListener::WebcamDeviceInAdded, d);
    return d;
}

void WebcamManager::removeInputDevice(WebcamInputDevice* d)
{
	inputs.removeObject(d, false);

	NLOG("Webcam", "Device In Removed : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&WebcamManagerListener::WebcamDeviceInRemoved, d);
	delete d;
}


WebcamInputDevice* WebcamManager::getInputDeviceWithName(String name)
{
	for (auto& d : inputs) if (d->name == name) return d;
	return nullptr;
}

void WebcamManager::run()
{
    while (!threadShouldExit()) {
        checkDevices();
        wait(1000);
    }
}