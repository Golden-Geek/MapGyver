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


	IntParameter* index;
	FloatParameter* defaultTransitionTime;

	MediaListItemManager listManager;

	WeakReference<Media> currentMedia;

	void updateMediaLoads();

	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void preRenderGLInternal() override;
	void renderGLInternal() override;


	void afterLoadJSONDataInternal() override;

	DECLARE_TYPE("Media List")
};