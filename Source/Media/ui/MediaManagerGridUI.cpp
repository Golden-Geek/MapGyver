/*
  ==============================================================================

    MediaManagerGridUI.cpp
    Created: 28 Aug 2024 11:46:57am
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaManagerGridUI.h"

MediaManagerGridUI::MediaManagerGridUI(const String& name) :
	BaseManagerShapeShifterUI(name, MediaManager::getInstance())
{
	highlightOnDragOver = false;

	contentIsFlexible = true;
	animateItemOnAdd = false;

	transparentBG = true;

	thumbSizeUI.reset(manager->gridThumbSize->createSlider());
	addAndMakeVisible(thumbSizeUI.get());
	thumbSizeUI->useCustomBGColor = true;
	thumbSizeUI->customBGColor = BG_COLOR.darker(.2f);

	setShowSearchBar(true);

	addExistingItems();

	manager->addAsyncContainerListener(this);

}

MediaManagerGridUI::~MediaManagerGridUI()
{
	if (!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
}

void MediaManagerGridUI::paint(Graphics& g)
{
	BaseManagerUI::paint(g);

	if (isDraggingOver)
	{
		g.setColour(BLUE_COLOR);

		if (MediaGridUI* bui = itemsUI[currentDropIndex >= 0 ? currentDropIndex : itemsUI.size() - 1])
		{
			juce::Rectangle<int> buiBounds = getLocalArea(bui, bui->getLocalBounds());
			int tx = currentDropIndex >= 0 ? buiBounds.getX() - 1 : buiBounds.getRight() + 1;
			g.drawLine(tx, buiBounds.getY(), tx, buiBounds.getBottom(), 2);
		}
	}
}

void MediaManagerGridUI::resizedInternalHeader(Rectangle<int>& r)
{
	BaseManagerShapeShifterUI::resizedInternalHeader(r);
	r.removeFromLeft(4);
	thumbSizeUI->setBounds(r.removeFromLeft(150).reduced(3));
	r.removeFromLeft(4);
}

void MediaManagerGridUI::resizedInternalContent(Rectangle<int>& r)
{
	viewport.setBounds(r);

	const int thumbSize = manager->gridThumbSize->floatValue();

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

int MediaManagerGridUI::getDropIndexForPosition(Point<int> localPosition)
{
	MediaGridUI* closestUI = nullptr;;
	float minDist = INT32_MAX;;
	bool rightSide = false;

	for (int i = 0; i < itemsUI.size(); ++i)
	{
		MediaGridUI* iui = itemsUI[i];
		Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();

		float dist = p.getDistanceFrom(localPosition);
		if (dist < minDist)
		{
			closestUI = iui;
			minDist = dist;
			rightSide = localPosition.x > p.x;
		}
	}


	int index = itemsUI.indexOf(closestUI);
	if (rightSide) index++;

	return index;
}

void MediaManagerGridUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		if (e.targetControllable == manager->gridThumbSize) resized();
	}
}
