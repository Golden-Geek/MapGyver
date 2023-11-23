/*
  ==============================================================================

	USBCamDeviceChooser.cpp
	Created: 20 Dec 2016 12:35:11pm
	Author:  Ben

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

USBCamDeviceChooser::USBCamDeviceChooser() :
	currentInputDevice(nullptr)
{
	addAndMakeVisible(&inputBox);

	setGhostValue("");

	inputBox.addListener(this);

	USBCamManager::getInstance()->addUSBCamManagerListener(this);

	updateInputComboBox();
}

USBCamDeviceChooser::~USBCamDeviceChooser()
{
	if (USBCamManager::getInstanceWithoutCreating() != nullptr)
	{
		USBCamManager::getInstance()->removeUSBCamManagerListener(this);
	}
}

void USBCamDeviceChooser::resized()
{
	Rectangle<int> r = getLocalBounds();
	//int th = (showInputs != showOutputs)?getHeight():getHeight() / 2 - 2;

	if (showInputs)
	{
		Rectangle<int> ir = r;
		//inputLabel.setBounds(ir.removeFromLeft(50));
		//r.removeFromLeft(10);
		inputBox.setBounds(ir);

		r.removeFromTop(4);
	}

}

void USBCamDeviceChooser::setGhostValue(const String& inValue)
{
	inputBox.setTextWhenNoChoicesAvailable(inValue.isEmpty() ? "No USBCam In Available" : "Disconnected : " + inValue);
	inputBox.setTextWhenNothingSelected(inValue.isEmpty() ? "No USBCam In Selected" : "Disconnected : " + inValue);
}

void USBCamDeviceChooser::updateInputComboBox()
{
	inputBox.clear(dontSendNotification);
	int index = 1;
	int idToSelect = 0;
	inputBox.addItem("Don't use input", -1);
	for (auto& i : USBCamManager::getInstance()->inputs)
	{
		if (currentInputDevice == i) idToSelect = index;
		inputBox.addItem(i->name, index);
		index++;
	}

	inputBox.setSelectedId(idToSelect, dontSendNotification);
}

void USBCamDeviceChooser::setSelectedInputDevice(USBCamInputDevice* i)
{
	inputBox.setSelectedId(USBCamManager::getInstance()->inputs.indexOf(i) + 1, dontSendNotification);
}

void USBCamDeviceChooser::setSelectedInputDevice(const String& deviceName)
{
	setSelectedInputDevice(USBCamManager::getInstance()->getInputDeviceWithName(deviceName));
}

void USBCamDeviceChooser::comboBoxChanged(ComboBox* cb)
{
	int deviceIndex = cb->getSelectedId() - 1;

	if (cb == &inputBox)
	{
		currentInputDevice = deviceIndex >= 0 ? USBCamManager::getInstance()->inputs[deviceIndex] : nullptr;
		chooserListeners.call(&ChooserListener::USBCamDeviceInSelected, currentInputDevice);
	}

}

void USBCamDeviceChooser::USBCamDeviceInAdded(USBCamInputDevice*)
{
	MessageManager::callAsync([this]() {updateInputComboBox(); });
}

void USBCamDeviceChooser::USBCamDeviceInRemoved(USBCamInputDevice*)
{
	MessageManager::callAsync([this](){updateInputComboBox(); });
}

