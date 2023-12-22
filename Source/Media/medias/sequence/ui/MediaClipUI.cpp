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

	fadeInHandle.setVisible(mediaClip->fadeIn->enabled);
	fadeOutHandle.setVisible(mediaClip->fadeOut->enabled);

	fadeInHandle.addMouseListener(this, false);
	fadeOutHandle.addMouseListener(this, false);

	bgColor = BG_COLOR.darker(.4f);

	if (dynamic_cast<ReferenceMediaClip*>(item) != nullptr) acceptedDropTypes.add("Media");

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
	LayerBlockUI::paint(g);
	g.setColour(bgColor.brighter());
	g.drawRoundedRectangle(getLocalBounds().toFloat(), 2, 1);

	g.setColour(TEXT_COLOR);
	g.setFont(g.getCurrentFont().withHeight(jlimit<float>(10, 20, getHeight() - 20)).boldened());
	g.drawText(mediaClip->niceName, getCoreBounds().toFloat(), Justification::centred);
}

void MediaClipUI::paintOverChildren(Graphics& g)
{
	LayerBlockUI::paintOverChildren(g);

	Colour fInColor = (mediaClip->fadeIn->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);
	Colour fOutColor = (mediaClip->fadeOut->enabled ? NORMAL_COLOR : BLUE_COLOR).withAlpha(.5f);

	if (mediaClip->fadeIn->floatValue() > 0)
	{
		g.setColour(fInColor);
		g.drawLine(0, getHeight(), getWidth() * (mediaClip->fadeIn->floatValue() / mediaClip->getTotalLength()), fadeInHandle.getY() + fadeInHandle.getHeight() / 2);
	}

	if (mediaClip->fadeOut->floatValue() > 0)
	{
		g.setColour(fOutColor);
		g.drawLine(getWidth() * (1 - (mediaClip->fadeOut->floatValue() / mediaClip->getTotalLength())), fadeOutHandle.getY() + fadeOutHandle.getHeight() / 2, getWidth(), getHeight());
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
}


void MediaClipUI::mouseDown(const MouseEvent& e)
{
	LayerBlockUI::mouseDown(e);

	if (e.eventComponent == &fadeInHandle) fadeValueAtMouseDown = mediaClip->fadeIn->floatValue();
	else if (e.eventComponent == &fadeOutHandle) fadeValueAtMouseDown = mediaClip->fadeOut->floatValue();

	if (e.mods.isRightButtonDown() && (e.eventComponent == this || e.eventComponent == automationUI.get()))
	{
		PopupMenu p;
		p.addItem(1, "Clear automation editor");

		PopupMenu ap;

		Array<WeakReference<Parameter>> params = mediaClip->media->mediaParams.getAllParameters(true);

		int index = 2;
		for (auto& pa : params)
		{
			if (pa->canBeAutomated) ap.addItem(index, pa->niceName, true, pa->controlMode == Parameter::ControlMode::AUTOMATION);
			index++;
		}

		p.addSubMenu("Edit...", ap);

		p.showMenuAsync(PopupMenu::Options(), [this, params](int result)
			{
				if (result > 0)
				{
					if (result == 1) setTargetAutomation(nullptr);
					else
					{
						WeakReference<Parameter> pa = params[result - 2];
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
					}
				}
			}
		);
	}
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
		OnlineContentItem* item = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get());
		if (item != nullptr)
		{
			if (Media* media = item->createMedia())
			{
				if (OwnedMediaClip* om = dynamic_cast<OwnedMediaClip*>(item))
				{
					om->setMedia(nullptr);
					om->ownedMedia.reset(media);
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
		bgColor = mediaClip->isActive->boolValue() ? GREEN_COLOR.darker() : BG_COLOR.darker(.4f);
		repaint();
	}
}

void MediaClipUI::newMessage(const MediaClip::MediaClipEvent& e)
{
	switch (e.type)
	{
	case MediaClip::MediaClipEvent::FADES_CHANGED:
		fadeInHandle.setVisible(mediaClip->fadeIn->enabled);
		fadeOutHandle.setVisible(mediaClip->fadeOut->enabled);
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
