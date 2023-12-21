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

	enum BlendMode { Alpha, Add, Multiply };
	EnumParameter* blendMode;

	OpenGLFrameBuffer frameBuffer;

	void initFrameBuffer(int width, int height);
	void renderFrameBuffer(int width, int height);
	void renderGL();

	void sequenceCurrentTimeChanged(Sequence* s, float prevTime, bool evaluateSkippedData) override;
	void sequencePlayStateChanged(Sequence* s) override;

	SequenceLayerTimeline* getTimelineUI() override;

	DECLARE_TYPE("Media")
	static MediaLayer* create(Sequence* s, var params = var()) { return new MediaLayer(s, params); }
};