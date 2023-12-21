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

	void renderGL();
};

class SequenceMedia :
	public Media
{
public:
	SequenceMedia(var params = var());
	~SequenceMedia();

	RMPSequence sequence;

	void renderGLInternal() override;

	DECLARE_TYPE("Sequence")
};