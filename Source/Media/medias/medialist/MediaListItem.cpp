/*
  ==============================================================================

	MediaListItem.cpp
	Created: 18 Feb 2026 8:24:02am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaListItem.h"


#define MEDIALISTITEM_MEDIA_ID 0
#define MEDIALISTITEM_TRANSITION_ID 1

MediaListItem::MediaListItem(const String& name, var params) :
	BaseItem(name),
	media(nullptr)
{
	saveAndLoadRecursiveData = true;
	canBeDisabled = true;

	itemDataType = "MediaListItem";

	transitionTime = addFloatParameter("Transition time", "In seconds", 1, 0);
	transitionTime->canBeDisabledByUser = true;
	transitionTime->setEnabled(false);
	transitionTime->defaultUI = FloatParameter::TIME;

	weight = addFloatParameter("Weight", "", 0, 0, 1);
	weight->setControllableFeedbackOnly(true);

	state = addEnumParameter("State", "");
	state->addOption("Idle", IDLE)->addOption("Loading", LOADING)->addOption("Unloading", UNLOADING);
	state->setControllableFeedbackOnly(true);

	shaderMedia.reset(new ShaderMedia());
	shaderMedia->setNiceName("Transition Shader");
	addChildControllableContainer(shaderMedia.get());
	shaderMedia->editorIsCollapsed = true;

	//registerUseMedia(MEDIALISTITEM_TRANSITION_ID, shaderMedia.get());
}

MediaListItem::~MediaListItem()
{
}

void MediaListItem::clearItem()
{
	setMedia(nullptr);
	shaderMedia.reset();
	MediaTarget::clearTarget();
	BaseItem::clearItem();
}

void MediaListItem::load(float fadeInTime)
{
	if (media == nullptr) return;
	if (weight->floatValue() == 1.f)
	{
		unregisterUseMedia(MEDIALISTITEM_MEDIA_ID);
		state->setValueWithData(IDLE);
		return;
	}

	registerUseMedia(MEDIALISTITEM_MEDIA_ID, media);
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

	registerUseMedia(MEDIALISTITEM_MEDIA_ID, media);
	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	weightAtStart = weight->floatValue();
	targetTime = timeAtStart + fadeOutTime * weightAtStart;
	targetWeight = 0.f;
	state->setValueWithData(UNLOADING);
}

void MediaListItem::process()
{
	TransitionState ts = state->getValueDataAsEnum<TransitionState>();
	switch (ts)
	{
	case IDLE:
		break;

	case LOADING:
	case UNLOADING:
	{
		double t = Time::getMillisecondCounterHiRes() / 1000.0;
		if (t >= targetTime)
		{
			weight->setValue(targetWeight);
			state->setValueWithData(IDLE);
		}
		else
		{
			double tWeight = jmap<double>(t, timeAtStart, targetTime, weightAtStart, targetWeight);
			weight->setValue(tWeight);
		}
	};
	}
}


void MediaListItem::onContainerParameterChangedInternal(Parameter* p)
{

}

void MediaListItem::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (media != nullptr && (c == media->width || c == media->height))
	{
		//size->setPoint(media->getMediaSize().toFloat());
	}
}

void MediaListItem::setMedia(Media* m)
{
	if (media == m) return;

	if (media != nullptr)
	{
		unregisterUseMedia(COMPOSITION_TARGET_MEDIA_ID);
	}

	media = m;


	if (media != nullptr)
	{
		registerUseMedia(COMPOSITION_TARGET_MEDIA_ID, m);
		//size->setPoint(media->getMediaSize().toFloat());
	}
}

bool MediaListItem::isUsingMedia(Media* m)
{
	if (!enabled->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
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
		removeChildControllableContainer(ownedMedia.get());
	}

	MediaListItem::setMedia(m);
	ownedMedia.reset(m);

	if (ownedMedia != nullptr)
	{
		addChildControllableContainer(ownedMedia.get());
	}
}
