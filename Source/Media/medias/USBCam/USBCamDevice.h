/*
  ==============================================================================

    USBCamDevice.h
    Created: 20 Dec 2016 1:17:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class USBCamDevice
{
public:
	enum Type { USBCam_IN, USBCam_OUT };
	USBCamDevice(String &deviceName, Type t);
	virtual ~USBCamDevice() {}

	String id;
	String name;
	Type type;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(USBCamDevice)
};

class USBCamInputDevice :
	public USBCamDevice,
	public CameraDevice::Listener
{
public:
	USBCamInputDevice(String &name);
	~USBCamInputDevice();
	//std::unique_ptr<USBCamInput> device;

	// Inherited via USBCamInputCallback
	//virtual void handleIncomingUSBCamMessage(USBCamInput * source, const USBCamMessage & message) override;

	CameraDevice* device;

	bool shouldProcess = false;

	class USBCamInputListener
	{
	public:
		/** Destructor. */
		virtual ~USBCamInputListener() {}
		virtual void usbCamImageReceived(const Image& image) {}
		//virtual void noteOnReceived(const int&/*channel*/, const int&/*pitch*/, const int&/*velocity*/) {}
	};

	ListenerList<USBCamInputListener> inputListeners;
	void addUSBCamInputListener(USBCamInputListener* newListener);
	void removeUSBCamInputListener(USBCamInputListener* listener);

	void imageReceived(const Image& image) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(USBCamInputDevice)

};
