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

//void RMPSequence::renderGL(int width, int height)
//{
//
//}


SequenceMedia::SequenceMedia(var params)
	: Media(getTypeString(), params, true)
{
	sequence.addSequenceListener(this);
	addChildControllableContainer(&sequence);
}

SequenceMedia::~SequenceMedia()
{
}

void SequenceMedia::renderGLInternal()
{
	Init2DViewport(width->intValue(), height->intValue());
	glColor3f(0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//sequence.renderGL(width->intValue(), height->intValue());

	frameBuffer.releaseAsRenderingTarget();

	Array<MediaLayer*> mediaLayers = sequence.layerManager->getItemsWithType<MediaLayer>();
	for (int i = mediaLayers.size() - 1; i >= 0; i--) //reverse so first in list is the last one processed
	{
		mediaLayers[i]->renderFrameBuffer(width->intValue(), height->intValue()); //generate framebuffers

		frameBuffer.makeCurrentRenderingTarget();
		mediaLayers[i]->renderGL(-i);
		frameBuffer.releaseAsRenderingTarget();

	}
}

void SequenceMedia::sequenceCurrentTimeChanged(Sequence* sequence, float time, bool evaluateSkippedData)
{
	shouldRedraw = true;
}
