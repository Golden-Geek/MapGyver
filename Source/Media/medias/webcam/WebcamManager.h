/*
  ==============================================================================

    WebcamManager.h
    Created: 20 Dec 2016 12:33:33pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WebcamManager :
	public Thread
{
public:
	juce_DeclareSingleton(WebcamManager,true)
	WebcamManager();
	~WebcamManager();

	OwnedArray<WebcamInputDevice> inputs;

	void checkDevices();
	WebcamInputDevice* addInputDeviceIfNotThere(String name);
	void removeInputDevice(WebcamInputDevice * d);

	WebcamInputDevice * getInputDeviceWithName(String name);

	class WebcamManagerListener
	{
	public:
		/** Destructor. */
		virtual ~WebcamManagerListener() {}
		virtual void WebcamDeviceInAdded(WebcamInputDevice* /*input*/) {}
		virtual void WebcamDeviceInRemoved(WebcamInputDevice* /*input*/) {}
	};

	ListenerList<WebcamManagerListener> listeners;
	void addWebcamManagerListener(WebcamManagerListener* newListener) { listeners.add(newListener); }
	void removeWebcamManagerListener(WebcamManagerListener* listener) { listeners.remove(listener); }



	// Inherited via Timer
	virtual void run() override;
	
	JUCE_DECLARE_NON_COPYABLE(WebcamManager)
};
