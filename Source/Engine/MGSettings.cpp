/*
  ==============================================================================

	MGSettings.cpp
	MapGyver-specific application settings

  ==============================================================================
*/

#include "JuceHeader.h"
#include "MGSettings.h"

juce_ImplementSingleton(MGSettings)

MGSettings::MGSettings() :
	ControllableContainer("MapGyver Settings"),
	mediaCC("Media")
{
	saveAndLoadRecursiveData = true;  // Enable persistence for settings

	// Video playback engine selection
	videoPlaybackEngine = mediaCC.addEnumParameter("Video Playback Engine", "Choose the video playback engine");
	videoPlaybackEngine->addOption("MPV", ENGINE_MPV);
#ifdef VLC_ENABLE
	videoPlaybackEngine->addOption("VLC", ENGINE_VLC);
#endif
	videoPlaybackEngine->setValueWithKey("MPV");

	addChildControllableContainer(&mediaCC);
}

MGSettings::~MGSettings()
{
}
