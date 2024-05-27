/*
  ==============================================================================

	MediaClipUI.cpp
	Created: 21 Dec 2023 10:40:59am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaClipUI.h"

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
	//LayerBlockUI::paint(g);

	g.setColour(bgColor);
	g.fillPath(clipPath);

	if (mediaClip->isActive->boolValue())
	{
		if (auto t = dynamic_cast<ClipTransition*>(mediaClip))
		{
			Rectangle<float> pathBounds = clipPath.getBounds();
			g.setColour(bgColor.brighter().withAlpha(.7f));
			g.fillRoundedRectangle(pathBounds.withWidth(pathBounds.getWidth() * t->progressParam->floatValue()), 2);
		}
	}


	if (mediaClip->media != nullptr)
	{
		double length = mediaClip->media->getMediaLength();
		if (length > 0)
		{
			int w = getCoreWidth() * (length / mediaClip->coreLength->doubleValue());
			g.setColour(BLUE_COLOR.withAlpha(.5f));
			g.fillRoundedRectangle(getLocalBounds().withWidth(jmin(getCoreWidth(), w)).toFloat(), 2);
		}
	}

	g.setColour(TEXT_COLOR);
	g.setFont(g.getCurrentFont().withHeight(jlimit<float>(10, 16, getHeight() - 20)).boldened());
	g.drawText(mediaClip->niceName, getCoreBounds().withLeft(usableLeft).withRight(usableRight).toFloat(), Justification::centred);
}

void MediaClipUI::paintOverChildren(Graphics& g)
{
	//LayerBlockUI::paintOverChildren(g);

	if (mediaClip->inTransition == nullptr)
	{
		Colour fInColor = (mediaClip->fadeIn->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);

		if (mediaClip->fadeIn->floatValue() > 0)
		{
			g.setColour(fInColor);
			g.drawLine(0, getHeight(), getWidth() * (mediaClip->fadeIn->floatValue() / mediaClip->getTotalLength()), fadeInHandle.getY() + fadeInHandle.getHeight() / 2);
		}
	}

	if (mediaClip->outTransition == nullptr)
	{
		Colour fOutColor = (mediaClip->fadeOut->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);
		if (mediaClip->fadeOut->floatValue() > 0)
		{
			g.setColour(fOutColor);
			g.drawLine(getWidth() * (1 - (mediaClip->fadeOut->floatValue() / mediaClip->getTotalLength())), fadeOutHandle.getY() + fadeOutHandle.getHeight() / 2, getWidth(), getHeight());
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

	fadeInHandle.setCentrePosition((mediaClip->fadeIn->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeInHandle.getHeight() / 2);
	fadeOutHandle.setCentrePosition((1 - mediaClip->fadeOut->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeOutHandle.getHeight() / 2);

	generatePath();
}

void MediaClipUI::generatePath()
{
	const int margin = 20;
	usableLeft = 0;
	usableRight = getWidth();


	if (ClipTransition* t = dynamic_cast<ClipTransition*>(mediaClip))
	{
		clipPath.clear();
		clipPath.addRoundedRectangle(getLocalBounds().toFloat().reduced(0, margin + 2), 2);
		return;
	}


	bool inTransitionOverlap = mediaClip->inTransition != nullptr && mediaClip->inTransition->getEndTime() > mediaClip->time->floatValue();
	bool outTransitionOverlap = mediaClip->outTransition != nullptr && mediaClip->outTransition->getEndTime() > mediaClip->time->floatValue();


	Path path;

	Rectangle<float> r = getLocalBounds().toFloat();

	LayerBlockManagerUI* mui = dynamic_cast<LayerBlockManagerUI*>(getParentComponent());



	if ((!inTransitionOverlap && !outTransitionOverlap) || mui == nullptr)
	{
		clipPath.clear();
		clipPath.addRoundedRectangle(r, 2);
		return;
	}


	path.startNewSubPath(0, getHeight());

	if (inTransitionOverlap)
	{
		usableLeft = mui->timeline->getXForTime(mediaClip->inTransition->getEndTime()) - getX() + 2;
		path.lineTo(0, getHeight() - margin);
		path.lineTo(usableLeft, getHeight() - margin);
		path.lineTo(usableLeft, 0);
	}
	else
	{
		path.lineTo(0, 0);
	}

	path.lineTo(getWidth(), 0);

	if (outTransitionOverlap)
	{
		usableRight = mui->timeline->getXForTime(mediaClip->outTransition->time->floatValue()) - getX() - 2;
		path.lineTo(getWidth(), margin);
		path.lineTo(usableRight, margin);
		path.lineTo(usableRight, getHeight());
	}
	else
	{
		path.lineTo(getWidth(), getHeight());
	}

	path.lineTo(0, getHeight());
	path.closeSubPath();
	clipPath = path.createPathWithRoundedCorners(2);
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

	if (c == mediaClip->fadeIn) fadeInHandle.setCentrePosition((mediaClip->fadeIn->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeInHandle.getHeight() / 2);
	else if (c == mediaClip->fadeOut) fadeOutHandle.setCentrePosition((1 - mediaClip->fadeOut->floatValue() / mediaClip->getTotalLength()) * getWidth(), fadeOutHandle.getHeight() / 2);
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
		fadeInHandle.setVisible(mediaClip->fadeIn->enabled && mediaClip->inTransition == nullptr);
		fadeOutHandle.setVisible(mediaClip->fadeOut->enabled && mediaClip->outTransition == nullptr);
		resized();
		repaint();
		break;

	case MediaClip::MediaClipEvent::REGENERATE_PREVIEW:
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
