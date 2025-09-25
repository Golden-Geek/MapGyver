/*
  ==============================================================================

	SequenceMedia.cpp
	Created: 21 Dec 2023 10:39:37am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MGSequence::MGSequence()
{
	layerManager->factory.defs.add(SequenceLayerManager::LayerDefinition::createDef("", "Media", &MediaLayer::create, this));
	layerManager->factory.defs.add(SequenceLayerManager::LayerDefinition::createDef("", "Audio", &MGAudioLayer::create, this, true));

}

String MGSequence::getPanelName() const
{
	if (Media* m = dynamic_cast<Media*>(parentContainer.get())) return m->niceName;
	return niceName;
}


SequenceMedia::SequenceMedia(var params)
	: Media(getTypeString(), params, true)
{
	sequence.addSequenceListener(this);
	sequence.setAudioDeviceManager(&AudioManager::getInstance()->am);
	addChildControllableContainer(&sequence);
	alwaysRedraw = true;

	sequence.layerManager->addManagerListener(this);
}

SequenceMedia::~SequenceMedia()
{
}

void SequenceMedia::itemAdded(SequenceLayer* layer)
{
	if (MediaLayer* ml = dynamic_cast<MediaLayer*>(layer))
	{
		addFrameBuffer(ml->niceName, &ml->frameBuffer);
		ml->addControllableContainerListener(this);
	}
}

void SequenceMedia::itemRemoved(SequenceLayer* layer)
{
	if (MediaLayer* ml = dynamic_cast<MediaLayer*>(layer))
	{
		removeFrameBuffer(ml->niceName);
		ml->removeControllableContainerListener(this);
	}
}

void SequenceMedia::controllableContainerNameChanged(ControllableContainer* cc, const String& oldName)
{
	if (cc->parentContainer.get() == sequence.layerManager.get())
	{
		if (MediaLayer* ml = dynamic_cast<MediaLayer*>(cc))
		{
			removeFrameBuffer(oldName);
			addFrameBuffer(ml->niceName, &ml->frameBuffer);
		}
	}
}

void SequenceMedia::renderGLInternal()
{
	Init2DViewport(width->intValue(), height->intValue());
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//sequence.renderGL(width->intValue(), height->intValue());
	glEnable(GL_BLEND);

	frameBuffer.releaseAsRenderingTarget();

	Array<MediaLayer*> mediaLayers = sequence.layerManager->getItemsWithType<MediaLayer>();
	for (int i = mediaLayers.size() - 1; i >= 0; i--) //reverse so first in list is the last one processed
	{
		if (!mediaLayers[i]->enabled->boolValue()) continue;

		GenericScopedLock<SpinLock> lock(mediaLayers[i]->renderLock);
		bool hasContent = mediaLayers[i]->renderFrameBuffer(width->intValue(), height->intValue()); //generate framebuffers

		if (!hasContent) continue;

		frameBuffer.makeCurrentRenderingTarget();
		mediaLayers[i]->renderGL(-i);
		frameBuffer.releaseAsRenderingTarget();
	}
	glDisable(GL_BLEND);
}

void SequenceMedia::sequenceCurrentTimeChanged(Sequence* sequence, float time, bool evaluateSkippedData)
{
	shouldRedraw = true;
}
