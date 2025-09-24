/*
  ==============================================================================

	SequenceMedia.h
	Created: 21 Dec 2023 10:39:37am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class MGSequence :
	public Sequence
{
public:
	MGSequence();

	String getPanelName() const override;
};

class SequenceMedia :
	public Media,
	public Sequence::SequenceListener,
	public SequenceLayerManager::ManagerListener
{
public:
	SequenceMedia(var params = var());
	~SequenceMedia();

	MGSequence sequence;

	void itemAdded(SequenceLayer* layer) override;
	void itemsAdded(Array<SequenceLayer*> layers) override { for (auto* l : layers) itemAdded(l); }
	void itemRemoved(SequenceLayer* layer) override;
	void itemsRemoved(Array<SequenceLayer*> layers) override { for (auto* l : layers) itemRemoved(l); }

	void controllableContainerNameChanged(ControllableContainer* cc, const String& oldName) override;


	void renderGLInternal() override;
	void sequenceCurrentTimeChanged(Sequence* sequence, float time, bool evaluateSkippedData) override;

	DECLARE_TYPE("Sequence")
};