/*
  ==============================================================================

	MediaListItem.cpp
	Created: 18 Feb 2026 8:24:02am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"


#define MEDIALISTITEM_MEDIA_ID 0
#define MEDIALISTITEM_TRANSITION_ID 1

MediaListItem::MediaListItem(const String& name, var params) :
	BaseItem(name),
	media(nullptr),
	listItemNotifier(10)
{
	saveAndLoadRecursiveData = true;
	canBeDisabled = true;

	itemDataType = "MediaListItem";

	launch = addTrigger("Launch", "Launch media with transition");

	transitionTime = addFloatParameter("Transition time", "In seconds", 1, 0);
	transitionTime->canBeDisabledByUser = true;
	transitionTime->setEnabled(false);
	transitionTime->defaultUI = FloatParameter::TIME;


	autoPlay = addBoolParameter("Auto play", "Whether to automatically play the media when loaded", true);
	autoStop = addBoolParameter("Auto stop", "Whether to automatically stop the media when unloaded", true);
	autoNextBehavior = addEnumParameter("Auto next", "Whether to automatically play the next media in the list when this media finishes or after a certain time");
	autoNextBehavior->addOption("Off", AUTO_NEXT_OFF)->addOption("Media finish", AUTO_NEXT_MEDIA_FINISH)->addOption("Force Time", AUTO_NEXT_TIME);
	autoNextTime = addFloatParameter("Auto next time", "Time in seconds before automatically playing the next media. if auto next ist set to Media Finish, this is 'pre-finish' time to allow smooth transition", 1, 0);
	autoNextTime->defaultUI = FloatParameter::TIME;

	weight = addFloatParameter("Weight", "", 0, 0, 1);
	weight->setControllableFeedbackOnly(true);



	state = addEnumParameter("State", "");
	state->addOption("Idle", IDLE)->addOption("Loading", LOADING)->addOption("Unloading", UNLOADING)->addOption("Running", RUNNING);
	state->setControllableFeedbackOnly(true);

	var manualRenderParams(new DynamicObject());
	manualRenderParams.getDynamicObject()->setProperty("manualRender", true);
	shaderMedia.reset(new ShaderMedia(manualRenderParams));


	shaderMedia->enabled->setValue(false);
	shaderMedia->setNiceName("Transition Shader");
	transitionProgression = shaderMedia->mediaParams.addFloatParameter("progression", "Progression", 0, 0, 1);
	transitionProgression->setControllableFeedbackOnly(true);
	transitionSourceMedia = shaderMedia->sourceMedias.addTargetParameter("Source", "Media");
	transitionSourceMedia->targetType = TargetParameter::CONTAINER;
	transitionSourceMedia->defaultContainerTypeCheckFunc = [](ControllableContainer* cc) { return dynamic_cast<Media*>(cc) != nullptr; };
	transitionTargetMedia = shaderMedia->sourceMedias.addTargetParameter("Target", "Media");
	transitionTargetMedia->targetType = TargetParameter::CONTAINER;
	transitionTargetMedia->defaultContainerTypeCheckFunc = [](ControllableContainer* cc) { return dynamic_cast<Media*>(cc) != nullptr; };


	addChildControllableContainer(shaderMedia.get());
	shaderMedia->editorIsCollapsed = true;
}

MediaListItem::~MediaListItem()
{
}

void MediaListItem::clearItem()
{
	setMedia(nullptr);
	shaderMedia->clearItem();
	MediaTarget::clearTarget();
	BaseItem::clearItem();
}

void MediaListItem::load(float fadeInTime, WeakReference<Media> prevMedia)
{
	if (media == nullptr) return;
	if (weight->floatValue() == 1.f)
	{
		state->setValueWithData(RUNNING);
		return;
	}

	if (shaderMedia->enabled->boolValue())
	{
		transitionProgression->setValue(0.f);
		transitionSourceMedia->setValueFromTarget(prevMedia);
		transitionTargetMedia->setValueFromTarget(media);
		forceRenderShader = true;
	}

	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	weightAtStart = weight->floatValue();
	targetTime = timeAtStart + fadeInTime * (1 - weightAtStart);
	targetWeight = 1.f;
	state->setValueWithData(LOADING);




}



void MediaListItem::unload(float fadeOutTime)
{
	if (media == nullptr) return;
	if (weight->floatValue() == 0.f)
	{
		state->setValueWithData(IDLE);
		return;
	}

	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	weightAtStart = weight->floatValue();
	targetTime = timeAtStart + fadeOutTime * weightAtStart;
	targetWeight = 0.f;
	state->setValueWithData(UNLOADING);

}

void MediaListItem::process()
{
	if (media == nullptr) return;

	if (forceRenderShader)
	{
		NLOG(niceName, "Force render shader");
		shaderMedia->renderOpenGLMedia(true);
		forceRenderShader = false;
	}

	TransitionState ts = state->getValueDataAsEnum<TransitionState>();
	switch (ts)
	{
	case IDLE:
		break;

	case RUNNING:
	{
		AutoNextBehavior an = (AutoNextBehavior)autoNextBehavior->getValueDataAsEnum<int>();
		float timeAN = autoNextTime->floatValue();
		if (an != AUTO_NEXT_OFF && timeAN > 0)
		{
			if (an == AUTO_NEXT_MEDIA_FINISH)
			{
				timeAN = media->getMediaLength() - timeAN;
			}

			double timeSinceStart = Time::getMillisecondCounterHiRes() / 1000.0 - timeAtStart;
			if (timeSinceStart > timeAN)
			{
				listItemNotifier.addMessage(new MediaListItemEvent(MediaListItemEvent::AUTO_NEXT, this));
			}
		}
	}
	break;

	case LOADING:
	case UNLOADING:
	{

		double t = Time::getMillisecondCounterHiRes() / 1000.0;
		if (t >= targetTime)
		{
			weight->setValue(targetWeight);
			state->setValueWithData(targetWeight > 0.f ? RUNNING : IDLE);
		}
		else
		{
			double tWeight = jmap<double>(t, timeAtStart, targetTime, weightAtStart, targetWeight);
			weight->setValue(tWeight);
		}
	};
	}

	if (shaderMedia->enabled->boolValue())
	{
		float progression = 0.f;
		if (ts == LOADING)
			progression = jmap<float>(weight->floatValue(), weightAtStart, 1.f, 0.f, 1.f);
		transitionProgression->setValue(progression);
	}
}



void MediaListItem::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == state)
	{
		TransitionState ts = state->getValueDataAsEnum<TransitionState>();
		if (ts != IDLE)
		{
			registerUseMedia(MEDIALISTITEM_MEDIA_ID, media);
		}
		else
		{
			unregisterUseMedia(MEDIALISTITEM_MEDIA_ID);
		}

		if (shaderMedia->enabled->boolValue())
		{
			if (ts == LOADING)
			{
				registerUseMedia(MEDIALISTITEM_TRANSITION_ID, shaderMedia.get());
			}
			else {
				unregisterUseMedia(MEDIALISTITEM_TRANSITION_ID);
			}
		}

		if (media != nullptr)
		{
			if (ts == LOADING && autoPlay->boolValue())
				media->handleEnter(0, true);
			else if (ts == IDLE && autoStop->boolValue())
				media->handleStop();
		}
	}
}

void MediaListItem::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	BaseItem::onControllableFeedbackUpdateInternal(cc, c);

	if (media != nullptr && (c == media->width || c == media->height))
	{
		//size->setPoint(media->getMediaSize().toFloat());
	}

	if (c == shaderMedia->enabled)
	{
		if (shaderMedia->enabled->boolValue())
		{
			transitionTargetMedia->setValueFromTarget(media);
		}
		else
		{
		}
	}
}

void MediaListItem::setMedia(Media* m)
{
	if (media == m) return;

	if (media != nullptr)
	{
		media->removeAsyncMediaListener(this);
		unregisterUseMedia(MEDIALISTITEM_MEDIA_ID);
	}

	media = m;


	if (media != nullptr)
	{
		Point<int> mediaSize = media->getMediaSize();
		if (mediaSize.x > 0 && mediaSize.y > 0)
		{
			shaderMedia->width->setValue(mediaSize.getX());
			shaderMedia->height->setValue(mediaSize.getY());
		}
		media->addAsyncMediaListener(this);
	}

	transitionTargetMedia->setValueFromTarget(media);
}

bool MediaListItem::isUsingMedia(Media* m)
{
	if (!enabled->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
}

bool MediaListItem::isLoading() const {
	return state->getValueDataAsEnum<TransitionState>() == LOADING;
}

bool MediaListItem::isUnloading() const {
	return state->getValueDataAsEnum<TransitionState>() == UNLOADING;
}

void MediaListItem::newMessage(const Media::MediaEvent& event)
{
	if (event.type == Media::MediaEvent::MEDIA_FINISHED)
	{
		TransitionState ts = state->getValueDataAsEnum<TransitionState>();
		AutoNextBehavior an = (AutoNextBehavior)autoNextBehavior->getValueDataAsEnum<int>();
		if (ts == RUNNING && an == AUTO_NEXT_MEDIA_FINISH && autoNextTime->floatValue() == 0)
		{
			listItemNotifier.addMessage(new MediaListItemEvent(MediaListItemEvent::AUTO_NEXT, this));
		}
	}
	else if (event.type == Media::MediaEvent::MEDIA_CONTENT_CHANGED)
	{
		if (media != nullptr && shaderMedia != nullptr)
		{
			Point<int> mediaSize = media->getMediaSize();
			if (mediaSize.x > 0 && mediaSize.y > 0)
			{
				shaderMedia->width->setValue(mediaSize.getX());
				shaderMedia->height->setValue(mediaSize.getY());
			}
		}
	}
}

ReferenceMediaListItem::ReferenceMediaListItem(var params) :
	MediaListItem(getTypeString(), params)
{
	targetMedia = addTargetParameter("Media", "Media", MediaManager::getInstance());
	targetMedia->maxDefaultSearchLevel = 0;
	targetMedia->targetType = TargetParameter::CONTAINER;
}

ReferenceMediaListItem::~ReferenceMediaListItem()
{
}

void ReferenceMediaListItem::onContainerParameterChangedInternal(Parameter* p)
{
	MediaListItem::onContainerParameterChangedInternal(p);
	if (p == targetMedia)
	{
		setMedia(targetMedia->getTargetContainerAs<Media>());
	}
}

OwnedMediaListItem::OwnedMediaListItem(var params) :
	MediaListItem(params.getProperty("mediaType", "").toString(), params),
	ownedMedia(nullptr)
{
	Media* m = MediaManager::getInstance()->factory.create(params.getProperty("mediaType", "").toString());
	setMedia(m);
}

OwnedMediaListItem::~OwnedMediaListItem()
{
}

void OwnedMediaListItem::setMedia(Media* m)
{
	if (ownedMedia != nullptr)
	{
		ownedMedia->clearItem();
		removeChildControllableContainer(ownedMedia.get());
	}

	MediaListItem::setMedia(m);
	ownedMedia.reset(m);

	if (ownedMedia != nullptr)
	{
		addChildControllableContainer(ownedMedia.get());
	}
}
