/*
  ==============================================================================

    NDIManager.h
    Created: 20 Dec 2016 12:33:33pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include <Processing.NDI.Lib.h>

#ifdef _WIN32
#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x86.lib")
#endif // _WIN64
#endif // _WIN32

#include "NDIDevice.h"


class NDIManager :
	public Thread
{
public:
	juce_DeclareSingleton(NDIManager,true)
	NDIManager();
	~NDIManager();

	NDIlib_find_instance_t pNDI_find;
	HashMap<String, std::shared_ptr<NDIlib_source_t>> sources;
	OwnedArray<NDIInputDevice> inputs;

	void checkDevices();
	NDIInputDevice* addInputDeviceIfNotThere(NDIlib_source_t info);
	void removeInputDevice(NDIInputDevice * d);

	NDIInputDevice * getInputDeviceWithName(const String &name);

	class Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void NDIDeviceInAdded(NDIInputDevice* /*input*/) {}
		virtual void NDIDeviceInRemoved(NDIInputDevice* /*input*/) {}
	};

	ListenerList<Listener> listeners;
	void addNDIManagerListener(Listener* newListener) { listeners.add(newListener); }
	void removeNDIManagerListener(Listener* listener) { listeners.remove(listener); }



	// Inherited via Timer
	virtual void run() override;
	
	JUCE_DECLARE_NON_COPYABLE(NDIManager)
};
