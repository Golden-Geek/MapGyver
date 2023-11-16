/*
  ==============================================================================

	NDIManager.cpp
	Created: 20 Dec 2016 12:33:33pm
	Author:  Ben

  ==============================================================================
*/
#include "JuceHeader.h"
#include "mainIncludes.h"
#include "NDIManager.h"

juce_ImplementSingleton(NDIManager)

NDIManager::NDIManager()
{

	// ndiRouterDefaultType = dynamic_cast<BKEngine *>(Engine::mainEngine)->defaultBehaviors.addEnumParameter("NDI Router Ouput Type","Choose the default type when choosing a NDI Module as Router output");
	// ndiRouterDefaultType->addOption("Control Change", NDIManager::CONTROL_CHANGE)->addOption("Note On", NDIManager::NOTE_ON)->addOption("Note Off", NDIManager::NOTE_OFF);
    pNDI_find = NDIlib_find_create_v2();

	startTimer(5000); //check devices each half seconds
	checkDevices();
}

NDIManager::~NDIManager()
{
}

void NDIManager::checkDevices()
{
	//INPUTS
	LOG("searching for devices");
    uint32_t no_sources = 0;
    const NDIlib_source_t* p_sources = NULL;
    NDIlib_find_wait_for_sources(pNDI_find, 1000);
    p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);
    bool changed = false;
    Array<String> current;
    for (auto it = sources.begin(); it != sources.end(); it.next()) {
        current.add(it.getKey());
    }
    if (no_sources > 0) {
        for (uint32_t i = 0; i < no_sources; i++) {
            String name = String(p_sources[i].p_ndi_name) + " " + String(p_sources[i].p_url_address);
            std::shared_ptr<NDIlib_source_t> sourceCopy = std::make_shared<NDIlib_source_t>(p_sources[i]);
            if (!sources.contains(name)) { 
                NDIlib_source_t t = p_sources[i];
                addInputDeviceIfNotThere(t);
                changed = true; 
            }
            current.removeAllInstancesOf(name);
            sources.set(name, sourceCopy);
        }
    }
    if (current.size() > 0) { changed = true; }
    if (changed) {
        LOG("Changed !!!");
    }

}

void NDIManager::addInputDeviceIfNotThere(NDIlib_source_t info)
{
	NDIInputDevice* d = new NDIInputDevice(info);
	inputs.add(d);

	NLOG("NDI", "Device In Added : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::ndiDeviceInAdded, d);
}

void NDIManager::removeInputDevice(NDIInputDevice* d)
{
	inputs.removeObject(d, false);

	NLOG("NDI", "Device In Removed : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::ndiDeviceInRemoved, d);
	delete d;
}


NDIInputDevice* NDIManager::getInputDeviceWithName(const String& name)
{
	for (auto& d : inputs) if (d->name == name) return d;
	return nullptr;
}

void NDIManager::timerCallback()
{
	checkDevices();
}