/*
  ==============================================================================

    WebcamDeviceChooser.h
    Created: 20 Dec 2016 12:35:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WebcamDeviceChooser :
	public Component,
	public ComboBox::Listener,
	public WebcamManager::WebcamManagerListener
{
public:
	WebcamDeviceChooser();
	~WebcamDeviceChooser();

	bool showInputs;
	//Label inputLabel;
	ComboBox inputBox;

	bool showOutputs;
	//Label outputLabel;
	ComboBox outputBox;

	WebcamInputDevice * currentInputDevice;

	void resized() override;

	void setGhostValue(const String &inValue);

	void updateInputComboBox();

	void setSelectedInputDevice(WebcamInputDevice * );
	void setSelectedInputDevice(const String &deviceName);

	virtual void comboBoxChanged(ComboBox * ccb) override;

	virtual void WebcamDeviceInAdded(WebcamInputDevice *) override;
	virtual void WebcamDeviceInRemoved(WebcamInputDevice *) override;

	class  ChooserListener
	{
	public:
		/** Destructor. */
		virtual ~ChooserListener() {}
		virtual void WebcamDeviceInSelected(WebcamInputDevice * /*input*/) {}
	};

	ListenerList<ChooserListener> chooserListeners;
	void addWebcamChooserListener(ChooserListener* newListener) { chooserListeners.add(newListener); }
	void removeWebcamChooserListener(ChooserListener* listener) { chooserListeners.remove(listener); }

};
