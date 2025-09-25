/*
  ==============================================================================

    MGAudioLayer.h
    Created: 24 Sep 2025 9:53:26pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class MGAudioLayer :
    public AudioLayer,
	public AudioManager::AudioManagerListener
{
public:
    MGAudioLayer(Sequence* sequence, var params);
    ~MGAudioLayer();

	void audioSetupChanged() override;

	static MGAudioLayer* create(Sequence* sequence, var params) { return new MGAudioLayer(sequence, params); }
};