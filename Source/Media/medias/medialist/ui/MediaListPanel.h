/*
  ==============================================================================

	MediaListPanel.h
	Created: 19 Feb 2026 9:59:21am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaListView :
	public ManagerUI<MediaListItemManager, MediaListItem, MediaListItemUI>,
	public ContainerAsyncListener
{
public:
	MediaListView(MediaListMedia* listMedia);
	~MediaListView();

	MediaListMedia* listMedia;

	std::unique_ptr<FloatSliderUI> thumbSizeSlider;
	std::unique_ptr<TriggerButtonUI> nextTrigger;
	std::unique_ptr<TriggerButtonUI> previousTrigger;

	void resizedInternalHeader(juce::Rectangle<int>& r) override;
	void resizedInternalContent(juce::Rectangle<int>& r) override;

	void newMessage(const ContainerAsyncEvent& e) override;
};

class MediaListPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::Listener,
	public Inspectable::InspectableListener
{
public:
	MediaListPanel(const String& name);
	~MediaListPanel() override;


	MediaListMedia* currentMedia;
	std::unique_ptr<MediaListView> mediaListView;

	void paint(Graphics& g) override;
	void resized() override;


	void setMediaList(MediaListMedia* media);


	void inspectablesSelectionChanged() override;
	void inspectableDestroyed(Inspectable* inspectable) override;

	static MediaListPanel* create(const String& name) { return new MediaListPanel(name); }
};
