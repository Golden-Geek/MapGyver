/*
  ==============================================================================

	SequenceMedia.cpp
	Created: 21 Dec 2023 10:39:37am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

RMPSequence::RMPSequence()
{
	layerManager->factory.defs.add(SequenceLayerManager::LayerDefinition::createDef("", "Media", &MediaLayer::create, this));
	layerManager->factory.defs.add(SequenceLayerManager::LayerDefinition::createDef("", "Audio", &AudioLayer::create, this, true));
}

void RMPSequence::renderGL()
{
	Array<MediaLayer*> mediaLayers = layerManager->getItemsWithType<MediaLayer>();
	for (int i = mediaLayers.size() - 1; i >= 0; i--) //reverse so first in list is the last one processed
	{
		mediaLayers[i]->renderGL();
	}
}


SequenceMedia::SequenceMedia(var params)
	: Media(getTypeString(), params, true)
{
	addChildControllableContainer(&sequence);
}

SequenceMedia::~SequenceMedia()
{
}

void SequenceMedia::renderGLInternal()
{
	sequence.renderGL();
}