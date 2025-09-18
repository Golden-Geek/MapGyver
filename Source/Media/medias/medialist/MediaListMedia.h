/*
  ==============================================================================

	MediaListMedia.h
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class MediaListMedia :
	public Media,
	public MediaTarget
{
public:
	MediaListMedia(var params = var());
	~MediaListMedia();


	ControllableContainer listCC;
	IntParameter* index;

	SpinLock mediaLock;
	Media* currentMedia;
	Point<int> currentMediaSize;

	void setCurrentMedia(Media* m);
	void setMediaFromIndex();

	void onContainerParameterChangedInternal(Parameter* c) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

		void renderGLInternal() override;

	Point<int> getMediaSize() override;

	void afterLoadJSONDataInternal() override;

	DECLARE_TYPE("MediaListMedia")
};