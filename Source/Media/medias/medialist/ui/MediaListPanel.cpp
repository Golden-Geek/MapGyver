/*
  ==============================================================================

	MediaListPanel.cpp
	Created: 19 Feb 2026 9:59:21am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"


MediaListView::MediaListView(MediaListMedia* listMedia) :
	ManagerUI(listMedia->niceName, &listMedia->listManager, true)
{
	addExistingItems();
	manager->addAsyncContainerListener(this);

	thumbSizeSlider.reset(listMedia->listManager.thumbSize->createSlider());
	nextTrigger.reset(listMedia->nextTrigger->createButtonUI());
	previousTrigger.reset(listMedia->previousTrigger->createButtonUI());
	addAndMakeVisible(thumbSizeSlider.get());
	addAndMakeVisible(nextTrigger.get());
	addAndMakeVisible(previousTrigger.get());
	headerSize = 24;
}

MediaListView::~MediaListView()
{
	if (inspectable.wasObjectDeleted()) return;
	manager->removeAsyncContainerListener(this);

}

void MediaListView::resizedInternalHeader(juce::Rectangle<int>& r)
{
	if (inspectable.wasObjectDeleted()) return;

	ManagerUI::resizedInternalHeader(r);

	previousTrigger->setBounds(r.removeFromLeft(100).reduced(2));
	nextTrigger->setBounds(r.removeFromLeft(100).reduced(2));
	r.removeFromLeft(8);
	thumbSizeSlider->setBounds(r.removeFromLeft(150).reduced(2));


}

void MediaListView::resizedInternalContent(juce::Rectangle<int>& r)
{
	if (inspectable.wasObjectDeleted()) return;
	viewport.setBounds(r);
	const int thumbSize = manager->thumbSize->floatValue();

	int numThumbs = getFilteredItems().size();
	int numThumbPerLine = jmax(1, jmin(r.getWidth() / (thumbSize + gap), numThumbs));
	int numLines = numThumbs == 0 ? 0 : ceil(numThumbs * 1.f / numThumbPerLine);

	Rectangle<int> cr;
	cr.setSize(r.getWidth(), numLines * (thumbSize + gap) - gap);
	container.setSize(cr.getWidth(), cr.getHeight());

	int index = 0;
	int yIndex = 0;

	Rectangle<int> lr;

	for (auto& mui : itemsUI)
	{
		if (!checkFilterForItem(mui))
		{
			mui->setVisible(false);
			continue;
		}

		if (index % numThumbPerLine == 0)
		{
			int numThumbsInThisLine = jmin(numThumbs - index, numThumbPerLine);
			int lineWidth = numThumbsInThisLine * (thumbSize + gap) - gap;

			if (yIndex > 0) cr.removeFromTop(gap);
			lr = cr.removeFromTop(thumbSize);
			lr = lr.withSizeKeepingCentre(lineWidth, lr.getHeight());

			yIndex++;
		}

		mui->setBounds(lr.removeFromLeft(thumbSize));
		lr.removeFromLeft(gap);
		index++;
	}

}

void MediaListView::newMessage(const ContainerAsyncEvent& e)
{
	if (inspectable.wasObjectDeleted()) return;
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate && e.targetControllable == manager->thumbSize)
	{
		resized();
	}
}


MediaListPanel::MediaListPanel(const String& name) :
	ShapeShifterContentComponent(name),
	currentMedia(nullptr)
{
	if (MediaListMedia* m = InspectableSelectionManager::mainSelectionManager->getInspectableAs<MediaListMedia>())
	{
		setMediaList(m);
	}

	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
}

MediaListPanel::~MediaListPanel()
{
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
	setMediaList(nullptr);
}

void MediaListPanel::paint(Graphics& g)
{
	ShapeShifterContentComponent::paint(g);
	g.setColour(Colours::darkgrey);
	g.drawText("No Grid Media Selected", getLocalBounds(), Justification::centred, true);
}

void MediaListPanel::resized()
{
	ShapeShifterContentComponent::resized();

	if (mediaListView != nullptr) mediaListView->setBounds(getLocalBounds());
}

void MediaListPanel::setMediaList(MediaListMedia* media)
{
	if (currentMedia == media) return;
	if (Engine::mainEngine->isClearing) return;
	 
	if (currentMedia != nullptr)
	{
		currentMedia->removeInspectableListener(this);
	}

	if (mediaListView != nullptr)
	{
		removeChildComponent(mediaListView.get());
		mediaListView.reset();
	}

	currentMedia = media;

	if (currentMedia != nullptr)
	{
		currentMedia->addInspectableListener(this);

		mediaListView.reset(new MediaListView(currentMedia));
		addAndMakeVisible(mediaListView.get());

		setCustomName("Grid Editor : " + currentMedia->niceName);
	}
	else
	{
		setCustomName("Grid Editor");
	}

	resized();
	repaint();
}

void MediaListPanel::inspectablesSelectionChanged()
{
	if (MediaListMedia* m = InspectableSelectionManager::mainSelectionManager->getInspectableAs<MediaListMedia>())
	{
		setMediaList(m);
	}
}

void MediaListPanel::inspectableDestroyed(Inspectable* inspectable)
{
	if (inspectable == currentMedia)
	{
		setMediaList(nullptr);
	}
}
