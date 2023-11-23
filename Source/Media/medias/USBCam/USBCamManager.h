/*
  ==============================================================================

    USBCamManager.h
    Created: 20 Dec 2016 12:33:33pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class USBCamManager :
	public Thread
{
public:
	juce_DeclareSingleton(USBCamManager,true)
	USBCamManager();
	~USBCamManager();

	OwnedArray<USBCamInputDevice> inputs;

	void checkDevices();
	USBCamInputDevice* addInputDeviceIfNotThere(String name);
	void removeInputDevice(USBCamInputDevice * d);

	USBCamInputDevice * getInputDeviceWithName(String name);

	class USBCamManagerListener
	{
	public:
		/** Destructor. */
		virtual ~USBCamManagerListener() {}
		virtual void USBCamDeviceInAdded(USBCamInputDevice* /*input*/) {}
		virtual void USBCamDeviceInRemoved(USBCamInputDevice* /*input*/) {}
	};

	ListenerList<USBCamManagerListener> listeners;
	void addUSBCamManagerListener(USBCamManagerListener* newListener) { listeners.add(newListener); }
	void removeUSBCamManagerListener(USBCamManagerListener* listener) { listeners.remove(listener); }



	// Inherited via Timer
	virtual void run() override;
	
	JUCE_DECLARE_NON_COPYABLE(USBCamManager)
};
