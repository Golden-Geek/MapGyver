/*
  ==============================================================================

    MGAudioLayer.cpp
    Created: 24 Sep 2025 9:53:26pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MGAudioLayer::MGAudioLayer(Sequence* sequence, var params) :
	AudioLayer(sequence, params)
{
	setAudioProcessorGraph(&AudioManager::getInstance()->graph, AUDIO_OUTPUTMIXER_GRAPH_ID);
	if (!Engine::mainEngine->isLoadingFile) updateSelectedOutChannels();
}

MGAudioLayer::~MGAudioLayer()
{
}
