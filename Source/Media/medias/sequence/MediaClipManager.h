/*
  ==============================================================================

	MediaClipManager.h
	Created: 21 Dec 2023 11:00:38am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaLayer;

class MediaClipManager :
	public LayerBlockManager,
	public MediaClip::MediaClipListener
{
public:
	MediaClipManager(MediaLayer* layer);
	~MediaClipManager();

	MediaLayer* mediaLayer;

	LayerBlock* createItem() override;

	void addItemInternal(LayerBlock* clip, var) override;
	void addItemsInternal(Array<LayerBlock*> clips, var) override;
	void removeItemInternal(LayerBlock* clip) override;
	void removeItemsInternal(Array<LayerBlock*> clips) override;

	void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	void mediaClipFadesChanged(MediaClip* block) override;
	void computeFadesForBlock(MediaClip* block, bool propagate);
};