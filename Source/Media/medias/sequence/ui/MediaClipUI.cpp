/*
  ==============================================================================

	MediaClipUI.cpp
	Created: 21 Dec 2023 10:40:59am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaClipUI::MediaClipUI(MediaClip* b) :
	LayerBlockUI(b),
	mediaClip(b),
	fadeValueAtMouseDown(0)
{
	addChildComponent(&fadeInHandle, 0);
	addChildComponent(&fadeOutHandle, 0);

	fadeInHandle.setVisible(mediaClip->fadeIn->enabled && mediaClip->inTransition == nullptr);
	fadeOutHandle.setVisible(mediaClip->fadeOut->enabled && mediaClip->outTransition == nullptr);

	fadeInHandle.addMouseListener(this, false);
	fadeOutHandle.addMouseListener(this, false);

	bgColor = BG_COLOR.darker(.4f);

	if (dynamic_cast<ReferenceMediaClip*>(item) != nullptr) acceptedDropTypes.add("Media");

	if (dynamic_cast<ClipTransition*>(mediaClip) != nullptr)
	{
		canBeGrabbed = false;
	}

	mediaClip->addAsyncMediaClipListener(this);
}

MediaClipUI::~MediaClipUI()
{
	if (!inspectable.wasObjectDeleted()) mediaClip->removeAsyncMediaClipListener(this);
}

void MediaClipUI::setTargetAutomation(ParameterAutomation* a)
{
	if (automationUI != nullptr)
	{
		removeChildComponent(automationUI.get());
		automationUI = nullptr;
	}

	canBeGrabbed = true;

	if (a == nullptr) return;


	if (dynamic_cast<ParameterNumberAutomation*>(a) != nullptr)
	{
		AutomationUI* aui = new AutomationUI((Automation*)a->automationContainer);
		//aui->updateROI();
		aui->showMenuOnRightClick = false;
		automationUI.reset(aui);
	}
	else if (dynamic_cast<ParameterColorAutomation*>(a) != nullptr)
	{
		GradientColorManagerUI* gui = new GradientColorManagerUI((GradientColorManager*)a->automationContainer);
		gui->autoResetViewRangeOnLengthUpdate = true;
		automationUI.reset(gui);
	}

	if (automationUI != nullptr)
	{
		canBeGrabbed = false;
		coreGrabber.setVisible(false);
		grabber.setVisible(false);
		loopGrabber.setVisible(false);
		automationUI->addMouseListener(this, true);
		addAndMakeVisible(automationUI.get());
		resized();
	}
}

void MediaClipUI::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;
	if (mediaClip->media == nullptr) return;

	g.setColour(bgColor);
	g.fillPath(clipPath);

	g.setColour(bgColor.darker());
	g.fillRect(getLoopBounds());

	if (mediaClip->media->previewImage.isValid())
	{
		Rectangle<float> usableBounds = usableCoreBounds.getUnion(usableLoopBounds);
		g.drawImage(mediaClip->media->previewImage, usableBounds.removeFromLeft(getHeight()).reduced(2).toFloat(), RectanglePlacement::onlyReduceInSize);
		g.drawImage(mediaClip->media->previewImage, usableBounds.removeFromRight(getHeight()).reduced(2).toFloat(), RectanglePlacement::onlyReduceInSize);
	}

	if (mediaClip->isActive->boolValue())
	{
		if (auto t = dynamic_cast<ClipTransition*>(mediaClip))
		{
			Rectangle<float> pathBounds = clipPath.getBounds();
			g.setColour(bgColor.brighter().withAlpha(.7f));
			g.fillRoundedRectangle(pathBounds.withWidth(pathBounds.getWidth() * t->progressParam->floatValue()), 2);
		}
	}

	g.setColour(TEXT_COLOR);
	g.setFont(g.getCurrentFont().withHeight(jlimit<float>(10, 16, getHeight() - 20)).boldened());
	g.drawText(mediaClip->niceName, usableCoreBounds.toFloat(), Justification::centred);

	if (mediaClip->loopLength->floatValue() > 0)
	{
		g.setColour(TEXT_COLOR.withAlpha(.6f));
		g.setFont(FontOptions(20));
		g.drawText("Loop", usableLoopBounds.toFloat(), Justification::centred);
	}

	if (viewStart > 0)
	{
		int tx = jmax(0, blockManagerUI->timeline->getXForTime(mediaClip->time->floatValue() + viewStart + 1)) - getX();
		tx = jmap<float>(jmin(viewStart / 1.f, 1.f), 0, tx);
		tx = jlimit<int>(0, 20, tx);
		g.setGradientFill(ColourGradient(YELLOW_COLOR.withAlpha(.5f), 0, getHeight() / 2, YELLOW_COLOR.withAlpha(.0f), tx, getHeight() / 2, false));

		g.fillRoundedRectangle(getLocalBounds().toFloat().removeFromLeft(tx), 2);
	}

	if (viewEnd < mediaClip->getTotalLength())
	{
		float diff = jmin(mediaClip->getTotalLength() - viewEnd, 1.f);
		int tx = getRealXForTime(viewEnd - diff);
		int tw = jmin(getWidth() - tx, 20);
		tx = getWidth() - tw;

		if (tw > 2)
		{
			g.setGradientFill(ColourGradient(YELLOW_COLOR.withAlpha(.0f), tx, getHeight() / 2, YELLOW_COLOR.withAlpha(.5f), getWidth(), getHeight() / 2, false));
			g.fillRoundedRectangle(getLocalBounds().toFloat().removeFromRight(tw), 2);
		}
	}
}

void MediaClipUI::paintOverChildren(Graphics& g)
{
	//LayerBlockUI::paintOverChildren(g);

	if (mediaClip->inTransition == nullptr)
	{
		Colour fInColor = (mediaClip->fadeIn->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);

		if (mediaClip->fadeIn->floatValue() > 0)
		{
			int startX = getRealXForTime(0);
			int endX = getRealXForTime(mediaClip->fadeIn->floatValue());

			if (endX > 0)
			{
				g.setColour(fInColor);
				g.drawLine(startX, getHeight(), endX, fadeInHandle.getY() + fadeInHandle.getHeight() / 2);
			}
		}
	}

	if (mediaClip->outTransition == nullptr)
	{
		Colour fOutColor = (mediaClip->fadeOut->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);
		if (mediaClip->fadeOut->floatValue() > 0)
		{
			int endX = getRealXForTime(mediaClip->getTotalLength());
			int startX = getRealXForTime(mediaClip->getTotalLength() - mediaClip->fadeOut->floatValue());

			if (startX < getWidth())
			{
				g.setColour(fOutColor);
				g.drawLine(startX, fadeOutHandle.getY() + fadeOutHandle.getHeight() / 2, endX, getHeight());
			}
		}
	}

	if (autoDrawContourWhenSelected && (inspectable->isSelected || inspectable->isPreselected))
	{
		g.setColour(inspectable->isSelected ? selectionContourColor : PRESELECT_COLOR);
		g.strokePath(clipPath, PathStrokeType(1));
	}
}

void MediaClipUI::resizedBlockInternal()
{
	if (automationUI != nullptr)
	{
		Rectangle<int> r = getCoreBounds();
		if (automationUI != nullptr)
		{
			if (dynamic_cast<GradientColorManagerUI*>(automationUI.get()) != nullptr) automationUI->setBounds(r.removeFromBottom(20));
			else automationUI->setBounds(r);
		}
	}

	int fadeInX = getRealXForTime(mediaClip->fadeIn->floatValue());
	fadeInHandle.setVisible(mediaClip->fadeIn->enabled && mediaClip->inTransition == nullptr && fadeInX > -fadeInHandle.getWidth() / 2);

	int fadeOutX = getRealXForTime(mediaClip->getTotalLength() - mediaClip->fadeOut->floatValue());
	fadeOutHandle.setVisible(mediaClip->fadeOut->enabled && mediaClip->outTransition == nullptr && fadeOutX < getWidth() + fadeOutHandle.getWidth() / 2);

	if (fadeInHandle.isVisible()) fadeInHandle.setCentrePosition(fadeInX, fadeInHandle.getHeight() / 2);
	if (fadeOutHandle.isVisible()) fadeOutHandle.setCentrePosition(fadeOutX, fadeOutHandle.getHeight() / 2);

	clipPath = generatePath();
	loopPath = generatePath(true);
}

Path MediaClipUI::generatePath(bool isLoop)
{
	const int margin = 20;
	Path path;


	usableCoreBounds.setBounds(0, 0, getWidth(), getHeight());

	if (ClipTransition* t = dynamic_cast<ClipTransition*>(mediaClip))
	{
		path.addRoundedRectangle(getLocalBounds().toFloat().reduced(0, margin + 2), 2);
		return path;
	}

	float tStart = item->time->floatValue();
	float tEnd = item->getEndTime();


	bool inTransitionOverlap = mediaClip->inTransition != nullptr && mediaClip->inTransition->getEndTime() > tStart;
	bool outTransitionOverlap = mediaClip->outTransition != nullptr && mediaClip->outTransition->time->floatValue() < tEnd;

	LayerBlockManagerUI* mui = dynamic_cast<LayerBlockManagerUI*>(getParentComponent());

	Rectangle<float> r = (isLoop ? getLoopBounds() : getLocalBounds()).toFloat();

	if ((!inTransitionOverlap && !outTransitionOverlap) || mui == nullptr)
	{
		path.addRoundedRectangle(r, 2);
		return path;
	}

	float h = getHeight();
	float hLow = getHeight() - getHeight() * .3f;
	float hHigh = getHeight() * .3f;


	float startX = mui->timeline->getXForTime(tStart);
	float endX = mui->timeline->getXForTime(tEnd);

	int leftOffset = 0;
	if (inTransitionOverlap)
	{
		int tEndX = mui->timeline->getXForTime(mediaClip->inTransition->getEndTime());
		leftOffset = tEndX - startX + 2;
	}

	int rightOffset = 0;
	if (outTransitionOverlap)
	{
		int tStartX = mui->timeline->getXForTime(mediaClip->outTransition->time->floatValue());
		rightOffset = endX - tStartX + 2;
	}

	usableCoreBounds.setLeft(leftOffset);
	usableCoreBounds.setRight(getWidth() - rightOffset);


	path.startNewSubPath(0, h);
	path.lineTo(0, hLow);
	path.lineTo(leftOffset, hLow);
	path.lineTo(leftOffset, 0);
	path.lineTo(getWidth(), 0);
	path.lineTo(getWidth(), hHigh);
	path.lineTo(getWidth() - rightOffset, hHigh);
	path.lineTo(getWidth() - rightOffset, h);
	path.closeSubPath();




	return path.createPathWithRoundedCorners(2);
}


void MediaClipUI::mouseDown(const MouseEvent& e)
{
	LayerBlockUI::mouseDown(e);

	if (e.eventComponent == &fadeInHandle) fadeValueAtMouseDown = mediaClip->fadeIn->floatValue();
	else if (e.eventComponent == &fadeOutHandle) fadeValueAtMouseDown = mediaClip->fadeOut->floatValue();
}

void MediaClipUI::mouseDrag(const MouseEvent& e)
{
	LayerBlockUI::mouseDrag(e);

	if (e.eventComponent == &fadeInHandle)
	{
		mediaClip->fadeIn->setValue((getMouseXYRelative().x * 1.0f / getWidth()) * mediaClip->getTotalLength());
		resized();
	}
	else if (e.eventComponent == &fadeOutHandle)
	{
		mediaClip->fadeOut->setValue((1 - (getMouseXYRelative().x * 1.0f / getWidth())) * mediaClip->getTotalLength());
		resized();
	}

	if (e.eventComponent == automationUI.get() && e.mods.isLeftButtonDown()) //because recursive mouseListener is removed to have special handling of automation
	{
		item->selectThis();
	}
}

void MediaClipUI::mouseUp(const MouseEvent& e)
{
	LayerBlockUI::mouseUp(e);

	if (e.eventComponent == &fadeInHandle)
	{
		mediaClip->fadeIn->setUndoableValue(fadeValueAtMouseDown, mediaClip->fadeIn->floatValue());
		resized();
	}
	else if (e.eventComponent == &fadeOutHandle)
	{
		mediaClip->fadeOut->setUndoableValue(fadeValueAtMouseDown, mediaClip->fadeOut->floatValue());
		resized();
	}
}

void MediaClipUI::addContextMenuItems(PopupMenu& m)
{
	m.addItem("Clear automation editor", automationUI == nullptr, false, [&] { setTargetAutomation(nullptr); });

	if (mediaClip->media != nullptr)
	{
		PopupMenu ap;
		Array<WeakReference<Parameter>> params = mediaClip->media->mediaParams.getAllParameters(true);

		int index = 2;
		for (auto& pa : params)
		{
			if (pa->canBeAutomated) ap.addItem(pa->niceName, true, pa->controlMode == Parameter::ControlMode::AUTOMATION, [&, pa]
				{
					if (pa->controlMode != Parameter::ControlMode::AUTOMATION)
					{
						pa->setControlMode(Parameter::ControlMode::AUTOMATION);
						pa->automation->setManualMode(true);
						Automation* a = dynamic_cast<Automation*>(pa->automation->automationContainer);
						if (a != nullptr)
						{
							a->clear();
							AutomationKey* k = a->addItem(0, 0);
							k->setEasing(Easing::BEZIER);
							a->addKey(a->length->floatValue(), 1);
						}
					}

					if (!pa.wasObjectDeleted()) setTargetAutomation(pa->automation.get());
				});
			index++;
		}
	}

	Array<MediaClip*> clips = InspectableSelectionManager::mainSelectionManager->getInspectablesAs<MediaClip>();
	clips.removeIf([this](MediaClip* c) { return dynamic_cast<ClipTransition*>(c) != nullptr; });

	clips.addIfNotAlreadyThere(mediaClip);

	bool canAddTransition = clips.size() == 2;
	MediaClip* inClip = nullptr;
	MediaClip* outClip = nullptr;

	if (canAddTransition)
	{
		inClip = clips[0]->time->floatValue() < clips[1]->time->floatValue() ? clips[0] : clips[1];
		outClip = inClip == clips[0] ? clips[1] : clips[0];
	}

	m.addSeparator();
	m.addItem("Create transition", canAddTransition, false, [&, inClip, outClip]() {
		ClipTransition* t = new ClipTransition();
		t->setInOutClips(inClip, outClip);
		((MediaClipManager*)item->parentContainer.get())->addBlockAt(t, inClip->getEndTime());
		});
}

bool MediaClipUI::hitTest(int x, int y)
{
	return clipPath.contains(Point<float>(x, y));
}

bool MediaClipUI::isInterestedInDragSource(const SourceDetails& source)
{
	if (source.description.getProperty("type", "") == "OnlineContentItem") return dynamic_cast<ReferenceMediaClip*>(item) == nullptr;
	return LayerBlockUI::isInterestedInDragSource(source);
}

void MediaClipUI::itemDropped(const SourceDetails& source)
{
	LayerBlockUI::itemDropped(source);

	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		if (OwnedMediaClip* om = dynamic_cast<OwnedMediaClip*>(item))
		{
			if (OnlineContentItem* contentItem = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get()))
			{
				if (Media* media = contentItem->createMedia())
				{
					om->setMedia(media);
				}
			}
		}
	}
	else
	{
		if (MediaUI* mui = dynamic_cast<MediaUI*>(source.sourceComponent.get()))
		{
			if (ReferenceMediaClip* rmc = dynamic_cast<ReferenceMediaClip*>(item))
			{
				rmc->mediaTarget->setValueFromTarget(mui->item);
			}
		}
	}

	repaint();
}

void MediaClipUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	LayerBlockUI::controllableFeedbackUpdateInternal(c);

	//if (c == mediaClip->fadeIn) fadeInHandle.setCentrePosition((mediaClip->fadeIn->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeInHandle.getHeight() / 2);
	//else if (c == mediaClip->fadeOut) fadeOutHandle.setCentrePosition((1 - mediaClip->fadeOut->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeOutHandle.getHeight() / 2);
	if (c == mediaClip->fadeIn || c == mediaClip->fadeOut) resized();
	else if (c == mediaClip->isActive)
	{
		bgColor = mediaClip->isActive->boolValue() ? GREEN_COLOR.darker(.6f) : BG_COLOR.darker(.4f);
		repaint();
	}
	else if (auto t = dynamic_cast<ClipTransition*>(mediaClip))
	{
		if (c == t->progressParam) repaint();
	}
}

void MediaClipUI::newMessage(const MediaClip::MediaClipEvent& e)
{
	switch (e.type)
	{
	case MediaClip::MediaClipEvent::FADES_CHANGED:
	case MediaClip::MediaClipEvent::TRANSITIONS_CHANGED:
		//fadeInHandle.setVisible(mediaClip->fadeIn->enabled && mediaClip->inTransition == nullptr);
		//fadeOutHandle.setVisible(mediaClip->fadeOut->enabled && mediaClip->outTransition == nullptr);
		resized();
		repaint();
		break;

	case MediaClip::MediaClipEvent::PREVIEW_CHANGED:
		repaint();
		break;

	}
}

MediaClipFadeHandle::MediaClipFadeHandle()
{
	setSize(12, 12);
}

void MediaClipFadeHandle::paint(Graphics& g)
{
	g.setColour(isMouseOverOrDragging() ? HIGHLIGHT_COLOR : NORMAL_COLOR);
	g.fillRoundedRectangle(getLocalBounds().reduced(3).toFloat(), 2);
}
