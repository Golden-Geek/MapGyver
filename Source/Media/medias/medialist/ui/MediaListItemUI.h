/*
  ==============================================================================

    MediaListItemUI.h
    Created: 19 Feb 2026 9:59:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaListItemUI :
    public ItemUI<MediaListItem>
{
public:
    MediaListItemUI(MediaListItem* item);
    ~MediaListItemUI() override;


	std::unique_ptr<TriggerButtonUI> triggerButton;
	Rectangle<int> transitionArea;
	Rectangle<int> previewArea;

	void paint(juce::Graphics& g) override;
	void resizedInternalHeader(juce::Rectangle<int>& r) override;
	void resizedInternalContent(juce::Rectangle<int>& r) override;

	void mouseDown(const juce::MouseEvent& event) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;

	void newMessage(const Inspectable::InspectableEvent& e) override;
};