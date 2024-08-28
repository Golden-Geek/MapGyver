/*
  ==============================================================================

	MediaGridUI.h
	Created: 28 Aug 2024 11:46:38am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaGridUI :
	public BaseItemUI<Media>
{
public:

	MediaGridUI(Media* item);
	virtual ~MediaGridUI();

	MediaPreview preview;

	void paintOverChildren(Graphics& g) override;
	void resizedInternalContent(Rectangle<int>& r) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;
};