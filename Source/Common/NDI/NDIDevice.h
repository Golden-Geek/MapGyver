/*
  ==============================================================================

    NDIDevice.h
    Created: 20 Dec 2016 1:17:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
struct NDIlib_source_t;

class NDIDevice
{
public:
	enum Type { NDI_IN, NDI_OUT };
	NDIDevice(NDIlib_source_t &deviceName, Type t);
	virtual ~NDIDevice() {}

	String id;
	String name;
	Type type;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NDIDevice)
};

class NDIInputDevice :
	public NDIDevice
{
public:
	NDIInputDevice(NDIlib_source_t &info);
	~NDIInputDevice();
	//std::unique_ptr<NDIInput> device;

	// Inherited via NDIInputCallback
	//virtual void handleIncomingNDIMessage(NDIInput * source, const NDIMessage & message) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NDIInputDevice)

};
