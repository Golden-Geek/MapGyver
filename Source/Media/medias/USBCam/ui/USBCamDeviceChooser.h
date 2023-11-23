/*
  ==============================================================================

    USBCamDeviceChooser.h
    Created: 20 Dec 2016 12:35:11pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class USBCamDeviceChooser :
	public Component,
	public ComboBox::Listener,
	public USBCamManager::USBCamManagerListener
{
public:
	USBCamDeviceChooser();
	~USBCamDeviceChooser();

	bool showInputs;
	//Label inputLabel;
	ComboBox inputBox;

	bool showOutputs;
	//Label outputLabel;
	ComboBox outputBox;

	USBCamInputDevice * currentInputDevice;

	void resized() override;

	void setGhostValue(const String &inValue);

	void updateInputComboBox();

	void setSelectedInputDevice(USBCamInputDevice * );
	void setSelectedInputDevice(const String &deviceName);

	virtual void comboBoxChanged(ComboBox * ccb) override;

	virtual void USBCamDeviceInAdded(USBCamInputDevice *) override;
	virtual void USBCamDeviceInRemoved(USBCamInputDevice *) override;

	class  ChooserListener
	{
	public:
		/** Destructor. */
		virtual ~ChooserListener() {}
		virtual void USBCamDeviceInSelected(USBCamInputDevice * /*input*/) {}
	};

	ListenerList<ChooserListener> chooserListeners;
	void addUSBCamChooserListener(ChooserListener* newListener) { chooserListeners.add(newListener); }
	void removeUSBCamChooserListener(ChooserListener* listener) { chooserListeners.remove(listener); }

};
