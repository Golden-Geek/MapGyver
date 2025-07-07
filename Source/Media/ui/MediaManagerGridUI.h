/*
  ==============================================================================

    MediaManagerGridUI.h
    Created: 28 Aug 2024 11:46:57am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaManagerGridUI :
	public ManagerShapeShifterUI<MediaManager, Media, MediaGridUI>,
	public ContainerAsyncListener
{
public:
	MediaManagerGridUI(const String& name);
	~MediaManagerGridUI();

	std::unique_ptr<IntSliderUI> thumbSizeUI;

	void paint(Graphics& g) override;

	void resizedInternalHeader(Rectangle<int>& r) override;
	void resizedInternalContent(Rectangle<int>& r) override;

	int getDropIndexForPosition(Point<int> localPosition) override;

	void newMessage(const ContainerAsyncEvent& e) override;

	static MediaManagerGridUI* create(const String& name) { return new MediaManagerGridUI(name); }
};