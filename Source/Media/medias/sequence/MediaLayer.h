/*
  ==============================================================================

    MediaLayer.h
    Created: 21 Dec 2023 10:39:51am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaLayer :
	public SequenceLayer
{
public:
	MediaLayer(Sequence* s, var params = var());
	~MediaLayer();

	MediaClipManager blockManager;

	void renderGL();

	SequenceLayerTimeline* getTimelineUI() override;

	DECLARE_TYPE("Media")
	static MediaLayer* create(Sequence* s, var params = var()) { return new MediaLayer(s, params); }
};