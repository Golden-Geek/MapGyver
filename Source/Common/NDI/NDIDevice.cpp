/*
  ==============================================================================

	NDIDevice.cpp
	Created: 20 Dec 2016 1:17:56pm
	Author:  Ben

  ==============================================================================
*/
#include "JuceHeader.h"
#include "NDIDevice.h"
#include "NDIManager.h"

NDIDevice::NDIDevice(NDIlib_source_t& info, Type t) :
	id(info.p_url_address),
	name(info.p_ndi_name),
	type(t)
{}



NDIInputDevice::NDIInputDevice(NDIlib_source_t & info) :
	NDIDevice(info, NDI_IN)
{
}

NDIInputDevice::~NDIInputDevice()
{
}



