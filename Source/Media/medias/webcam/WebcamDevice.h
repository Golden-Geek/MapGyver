/*
  ==============================================================================

    WebcamDevice.h
    Created: 20 Dec 2016 1:17:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WebcamDevice
{
public:
	enum Type { Webcam_IN, Webcam_OUT };
	WebcamDevice(String &deviceName, Type t);
	virtual ~WebcamDevice() {}

	String id;
	String name;
	Type type;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebcamDevice)
};

class WebcamInputDevice :
	public WebcamDevice,
	public CameraDevice::Listener
{
public:
	WebcamInputDevice(String &name);
	~WebcamInputDevice();
	//std::unique_ptr<WebcamInput> device;

	// Inherited via WebcamInputCallback
	//virtual void handleIncomingWebcamMessage(WebcamInput * source, const WebcamMessage & message) override;

	CameraDevice* device;

	bool shouldProcess = false;

	class WebcamInputListener
	{
	public:
		/** Destructor. */
		virtual ~WebcamInputListener() {}
		virtual void WebcamImageReceived(const Image& image) {}
		//virtual void noteOnReceived(const int&/*channel*/, const int&/*pitch*/, const int&/*velocity*/) {}
	};

	ListenerList<WebcamInputListener> inputListeners;
	void addWebcamInputListener(WebcamInputListener* newListener);
	void removeWebcamInputListener(WebcamInputListener* listener);

	void imageReceived(const Image& image) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebcamInputDevice)

};
