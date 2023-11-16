/*
  ==============================================================================

    NDIDevice.h
    Created: 20 Dec 2016 1:17:56pm
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
	public NDIDevice,
	public Thread
{
public:
	NDIInputDevice(NDIlib_source_t &info);
	~NDIInputDevice();
	//std::unique_ptr<NDIInput> device;

	NDIlib_recv_instance_t pNDI_recv;
	NDIlib_source_t* p_source;

	// Inherited via NDIInputCallback
	//virtual void handleIncomingNDIMessage(NDIInput * source, const NDIMessage & message) override;

	void run() override;
	bool shouldProcess = false;

	class  NDIInputListener
	{
	public:
		/** Destructor. */
		virtual ~NDIInputListener() {}
		//virtual void noteOnReceived(const int&/*channel*/, const int&/*pitch*/, const int&/*velocity*/) {}
	};

	ListenerList<NDIInputListener> inputListeners;
	void addNDIInputListener(NDIInputListener* newListener);
	void removeNDIInputListener(NDIInputListener* listener);


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NDIInputDevice)

};
