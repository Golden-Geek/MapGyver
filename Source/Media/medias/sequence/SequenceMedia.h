/*
  ==============================================================================

	SequenceMedia.h
	Created: 21 Dec 2023 10:39:37am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class RMPSequence :
	public Sequence
{
public:
	RMPSequence();

	String getPanelName() const override;
};

class SequenceMedia :
	public Media,
	public Sequence::SequenceListener
{
public:
	SequenceMedia(var params = var());
	~SequenceMedia();

	RMPSequence sequence;


	void renderGLInternal() override;
	void sequenceCurrentTimeChanged(Sequence* sequence, float time, bool evaluateSkippedData) override;

	DECLARE_TYPE("Sequence")
};