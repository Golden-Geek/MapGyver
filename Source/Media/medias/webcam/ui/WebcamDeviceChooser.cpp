/*
  ==============================================================================

	WebcamDeviceChooser.cpp
	Created: 20 Dec 2016 12:35:11pm
	Author:  Ben

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

WebcamDeviceChooser::WebcamDeviceChooser() :
	currentInputDevice(nullptr)
{
	addAndMakeVisible(&inputBox);

	setGhostValue("");

	inputBox.addListener(this);

	WebcamManager::getInstance()->addWebcamManagerListener(this);

	updateInputComboBox();
}

WebcamDeviceChooser::~WebcamDeviceChooser()
{
	if (WebcamManager::getInstanceWithoutCreating() != nullptr)
	{
		WebcamManager::getInstance()->removeWebcamManagerListener(this);
	}
}

void WebcamDeviceChooser::resized()
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

void WebcamDeviceChooser::setGhostValue(const String& inValue)
{
	inputBox.setTextWhenNoChoicesAvailable(inValue.isEmpty() ? "No Webcam In Available" : "Disconnected : " + inValue);
	inputBox.setTextWhenNothingSelected(inValue.isEmpty() ? "No Webcam In Selected" : "Disconnected : " + inValue);
}

void WebcamDeviceChooser::updateInputComboBox()
{
	inputBox.clear(dontSendNotification);
	int index = 1;
	int idToSelect = 0;
	inputBox.addItem("Don't use input", -1);
	for (auto& i : WebcamManager::getInstance()->inputs)
	{
		if (currentInputDevice == i) idToSelect = index;
		inputBox.addItem(i->name, index);
		index++;
	}

	inputBox.setSelectedId(idToSelect, dontSendNotification);
}

void WebcamDeviceChooser::setSelectedInputDevice(WebcamInputDevice* i)
{
	inputBox.setSelectedId(WebcamManager::getInstance()->inputs.indexOf(i) + 1, dontSendNotification);
}

void WebcamDeviceChooser::setSelectedInputDevice(const String& deviceName)
{
	setSelectedInputDevice(WebcamManager::getInstance()->getInputDeviceWithName(deviceName));
}

void WebcamDeviceChooser::comboBoxChanged(ComboBox* cb)
{
	int deviceIndex = cb->getSelectedId() - 1;

	if (cb == &inputBox)
	{
		currentInputDevice = deviceIndex >= 0 ? WebcamManager::getInstance()->inputs[deviceIndex] : nullptr;
		chooserListeners.call(&ChooserListener::WebcamDeviceInSelected, currentInputDevice);
	}

}

void WebcamDeviceChooser::WebcamDeviceInAdded(WebcamInputDevice*)
{
	MessageManager::callAsync([this]() {updateInputComboBox(); });
}

void WebcamDeviceChooser::WebcamDeviceInRemoved(WebcamInputDevice*)
{
	MessageManager::callAsync([this](){updateInputComboBox(); });
}

