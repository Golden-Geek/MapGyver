/*
  ==============================================================================

	MGAudioLayer.cpp
	Created: 24 Sep 2025 9:53:26pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MGAudioLayer.h"

MGAudioLayer::MGAudioLayer(Sequence* sequence, var params) :
	AudioLayer(sequence, params)
{
	AudioManager::getInstance()->addAudioManagerListener(this);
	setAudioProcessorGraph(&AudioManager::getInstance()->graph, AUDIO_OUTPUTMIXER_GRAPH_ID);
	if (!Engine::mainEngine->isLoadingFile) updateSelectedOutChannels();
}

MGAudioLayer::~MGAudioLayer()
{
	if (AudioManager::getInstanceWithoutCreating())
		AudioManager::getInstance()->removeAudioManagerListener(this);
}

void MGAudioLayer::audioSetupChanged()
{
	updateSelectedOutChannels();
}

int MGAudioLayer::getNodeGraphIDIncrement()
{
	return AudioManager::getInstance()->getUniqueNodeGraphID();
}
