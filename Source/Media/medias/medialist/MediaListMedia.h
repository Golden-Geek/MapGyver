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
	public MediaTarget,
	public MediaListItemManager::ManagerListener,
	public MediaListItem::AsyncListener
{
public:
	MediaListMedia(var params = var());
	~MediaListMedia();


	IntParameter* index;
	Trigger* previousTrigger;
	Trigger* nextTrigger;
	BoolParameter* loop;
	FloatParameter* defaultTransitionTime;

	MediaListItemManager listManager;

	WeakReference<Media> currentMedia;

	void clearItem() override;
	void updateMediaLoads();

	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void preRenderGLInternal() override;
	void renderGLInternal() override;

	void itemAdded(MediaListItem* item) override;
	void itemsAdded(juce::Array<MediaListItem*> items) override;
	void itemRemoved(MediaListItem* item) override;
	void itemsRemoved(juce::Array<MediaListItem*> items) override;

	void newMessage(const MediaListItem::MediaListItemEvent& e) override;

	void afterLoadJSONDataInternal() override;

	DECLARE_TYPE("Media List")
};