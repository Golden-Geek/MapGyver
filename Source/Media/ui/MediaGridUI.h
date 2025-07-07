/*
  ==============================================================================

	MediaGridUI.h
	Created: 28 Aug 2024 11:46:38am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaGridUIPreview : public MediaPreview
{
public:
	juce_DeclareSingleton(MediaGridUIPreview, true);
	MediaGridUIPreview() : MediaPreview()
	{
		setInterceptsMouseClicks(false, false);
	}
	~MediaGridUIPreview() {}
};

class MediaGridUI :
	public ItemUI<Media>,
	public Media::AsyncListener
{
public:

	MediaGridUI(Media* item);
	virtual ~MediaGridUI();
	

	bool useLivePreview;
	bool useLiveOnHover;

	Image previewImage;
	Rectangle<int> previewBounds;


	void mouseEnter(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;

	void setUseLivePreview(bool value);
	void updatePreview();

	void paint(Graphics& g) override;
	void resizedInternalContent(Rectangle<int>& r) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	void newMessage(const Media::MediaEvent& e) override;
};