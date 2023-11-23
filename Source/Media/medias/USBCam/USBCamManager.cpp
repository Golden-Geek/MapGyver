/*
  ==============================================================================

	USBCamManager.cpp
	Created: 20 Dec 2016 12:33:33pm
	Author:  Ben

  ==============================================================================
*/
#include "Media/MediaIncludes.h"

juce_ImplementSingleton(USBCamManager)

USBCamManager::USBCamManager() :
    Thread("USBCam Manager")
{
	// USBCamRouterDefaultType = dynamic_cast<BKEngine *>(Engine::mainEngine)->defaultBehaviors.addEnumParameter("USBCam Router Ouput Type","Choose the default type when choosing a USBCam Module as Router output");
	// USBCamRouterDefaultType->addOption("Control Change", USBCamManager::CONTROL_CHANGE)->addOption("Note On", USBCamManager::NOTE_ON)->addOption("Note Off", USBCamManager::NOTE_OFF);

    startThread();
}

USBCamManager::~USBCamManager()
{
    stopThread(3000);
}

void USBCamManager::checkDevices()
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
            USBCamInputDevice* input = getInputDeviceWithName(names[i]);
            if (input == nullptr) {
                input = addInputDeviceIfNotThere(names[i]);
            }
        }
    }
}

USBCamInputDevice* USBCamManager::addInputDeviceIfNotThere(String name)
{
	USBCamInputDevice* d = new USBCamInputDevice(name);
	inputs.add(d);

	NLOG("USBCam", "Device In Added : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&USBCamManagerListener::USBCamDeviceInAdded, d);
    return d;
}

void USBCamManager::removeInputDevice(USBCamInputDevice* d)
{
	inputs.removeObject(d, false);

	NLOG("USBCam", "Device In Removed : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&USBCamManagerListener::USBCamDeviceInRemoved, d);
	delete d;
}


USBCamInputDevice* USBCamManager::getInputDeviceWithName(String name)
{
	for (auto& d : inputs) if (d->name == name) return d;
	return nullptr;
}

void USBCamManager::run()
{
    while (!threadShouldExit()) {
        checkDevices();
        wait(1000);
    }
}