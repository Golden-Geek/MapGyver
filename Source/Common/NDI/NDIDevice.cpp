/*
  ==============================================================================

	NDIDevice.cpp
	Created: 20 Dec 2016 1:17:56pm
	Author:  Ben

  ==============================================================================
*/
#include "Common/CommonIncludes.h"

NDIDevice::NDIDevice(NDIlib_source_t& info, Type t) :
	id(info.p_url_address),
	name(info.p_ndi_name),
	type(t)
{}



NDIInputDevice::NDIInputDevice(NDIlib_source_t & info) :
	NDIDevice(info, NDI_IN),
	Thread("NDI Input")
{
	startThread();
}

NDIInputDevice::~NDIInputDevice()
{
	stopThread(100);
}



void NDIInputDevice::addNDIInputListener(NDIInputListener* newListener)
{
	inputListeners.add(newListener);
	if (inputListeners.size() == 1)
	{

		//int deviceIndex = MidiInput::getDevices().indexOf(name);
		pNDI_recv = NDIlib_recv_create_v3();
		// Connect to our sources
		NDIlib_recv_connect(pNDI_recv, p_source);

		// Destroy the NDI finder. We needed to have access to the pointers to p_sources[0]
		//NDIlib_find_destroy(pNDI_find);

		shouldProcess = true;
	}
}

void NDIInputDevice::removeNDIInputListener(NDIInputListener* listener) {
	inputListeners.remove(listener);
	if (inputListeners.size() == 0)
	{
		shouldProcess = false;
		NDIlib_recv_destroy(pNDI_recv);
		LOG("close connexion here");
	}
}

void NDIInputDevice::run()
{
	while (!threadShouldExit()) {
		if (shouldProcess) {
			NDIlib_video_frame_v2_t video_frame;
			NDIlib_audio_frame_v2_t audio_frame;

			switch (NDIlib_recv_capture_v2(pNDI_recv, &video_frame, nullptr, nullptr, 100)) {
				// No data
			case NDIlib_frame_type_none:
				//LOG("No data received");
				break;

				// Video data
			case NDIlib_frame_type_video:

				//LOG("Video data received : " <<  video_frame.xres << " / " << video_frame.yres);
				NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
				inputListeners.call(&NDIInputListener::videoFrameReceived, &video_frame);
				break;

				// Audio data
			case NDIlib_frame_type_audio:
				//LOG("Audio data received : " << audio_frame.no_samples);
				NDIlib_recv_free_audio_v2(pNDI_recv, &audio_frame);
				break;
			}
		}
		else {
			wait(1000);
		}
	}
}

