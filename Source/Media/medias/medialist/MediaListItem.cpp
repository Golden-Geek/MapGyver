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
	listItemNotifier(10)
{
	saveAndLoadRecursiveData = false;
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
	autoNextBehavior->addOption("Off", AUTO_NEXT_OFF)->addOption("Media finish (first)", AUTO_NEXT_MEDIA_FINISH_FIRST)->addOption("Media finish (last)", AUTO_NEXT_MEDIA_FINISH_LAST)->addOption("Force Time", AUTO_NEXT_TIME);
	autoNextTime = addFloatParameter("Auto next time", "Time in seconds before automatically playing the next media. if auto next ist set to Media Finish, this is 'pre-finish' time to allow smooth transition", 1, 0);
	autoNextTime->defaultUI = FloatParameter::TIME;

	weight = addFloatParameter("Weight", "", 0, 0, 1);
	weight->setControllableFeedbackOnly(true);



	state = addEnumParameter("State", "");
	state->addOption("Idle", IDLE)->addOption("Loading", LOADING)->addOption("Unloading", UNLOADING)->addOption("Running", RUNNING);
	state->setControllableFeedbackOnly(true);
}

MediaListItem::~MediaListItem()
{
}

void MediaListItem::setNumLayers(int num)
{
	while (num < subItems.size())
	{
		subItems.getLast()->removeAsyncMediaListSubItemListener(this);
		removeChildControllableContainer(subItems.getLast());
		subItems.removeLast();
	}

	while (num > subItems.size())
	{
		MediaListSubItem* newSubItem = new MediaListSubItem("Layer " + String(subItems.size() + 1), subItems.size() > 0);
		newSubItem->transitionTimeOverride->setRange(0, transitionTime->floatValue());
		subItems.add(newSubItem);
		addChildControllableContainer(newSubItem);
		newSubItem->addAsyncMediaListSubItemListener(this);
	}
}

void MediaListItem::clearItem()
{
	for (auto& subItem : subItems)
	{
		subItem->clear();
	}

	listItemNotifier.cancelPendingUpdate();
	listItemNotifier.clearQueue();

	BaseItem::clearItem();
}

void MediaListItem::load(float defaultTransitionTime, MediaListItem* prevMedia)
{
	if (weight->floatValue() == 1.f)
	{
		state->setValueWithData(RUNNING);
		return;
	}

	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	weightAtStart = weight->floatValue();
	float fadeInTime = transitionTime->enabled ? transitionTime->floatValue() : defaultTransitionTime;
	targetTime = timeAtStart + fadeInTime * (1 - weightAtStart);

	for (int i = 0; i < subItems.size(); i++)
	{
		MediaListSubItem* subItem = dynamic_cast<MediaListSubItem*>(subItems[i]);
		jassert(subItem != nullptr);
		subItem->setupTransition(prevMedia != nullptr ? prevMedia->getMediaAt(i) : nullptr);


		bool timeOverride = subItem->transitionTimeOverride->enabled;
		subItem->weightAtStart = subItem->weight;
		subItem->targetEndTransitionTime = timeOverride ? timeAtStart + subItem->transitionTimeOverride->floatValue() * (1 - subItem->weightAtStart) : targetTime;
	}

	targetWeight = 1.f;
	state->setValueWithData(LOADING);

}

void MediaListItem::unload(float fadeOutTime, Array<float> fadeOutSubTimes)
{
	if (weight->floatValue() == 0.f)
	{
		state->setValueWithData(IDLE);
		return;
	}

	timeAtStart = Time::getMillisecondCounterHiRes() / 1000.0;
	weightAtStart = weight->floatValue();
	targetTime = timeAtStart + fadeOutTime * weightAtStart;

	int i = 0;
	for (auto& subItem : subItems)
	{
		subItem->weightAtStart = subItem->weight;
		float subFadeOutTime = fadeOutSubTimes.size() > i ? fadeOutSubTimes[i] : fadeOutTime;
		subItem->targetEndTransitionTime = timeAtStart + subFadeOutTime * subItem->weightAtStart;
		i++;
	}

	targetWeight = 0.f;
	state->setValueWithData(UNLOADING);

}

void MediaListItem::render()
{
	if (weight->floatValue() == 0.f) return;

	for (auto& subItem : subItems)
	{
		subItem->render(isLoading());
	}

}

void MediaListItem::process()
{
	for (auto& subItem : subItems)
	{
		subItem->renderShaderIfNecessary();
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
		if (an != AUTO_NEXT_OFF && (timeAN > 0 || an == AUTO_NEXT_MEDIA_FINISH_LAST))
		{
			if (an == AUTO_NEXT_MEDIA_FINISH_FIRST || an == AUTO_NEXT_MEDIA_FINISH_LAST)
			{
				float referenceLength = getReferenceLength();
				timeAN = referenceLength - timeAN;
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
			for (auto& s : subItems)
			{
				s->weight = targetWeight;
			}
		}
		else
		{
			for (auto& s : subItems)
			{
				if (t >= s->targetEndTransitionTime)
					s->weight = targetWeight;
				else
					s->weight = jmap<double>(t, timeAtStart, s->targetEndTransitionTime, s->weightAtStart, targetWeight);
			}
			double tWeight = jmap<double>(t, timeAtStart, targetTime, weightAtStart, targetWeight);
			weight->setValue(tWeight);
		}
	};
	}

	for (auto& s : subItems)
	{
		if (!s->shaderMedia->enabled->boolValue()) continue;
		float progression = 0.f;
		if (ts == LOADING)
			progression = jmap<float>(s->weight, s->weightAtStart, 1.f, 0.f, 1.f);
		s->transitionProgression->setValue(progression);
	}

	render();
}

Media* MediaListItem::getMediaAt(int index)
{
	if (index < 0 || index >= subItems.size()) return nullptr;
	if (subItems[index]->isSubTexture() && index > 0)
	{
		return getMediaAt(0);
	}

	return subItems[index]->media;
}

OpenGLFrameBuffer* MediaListItem::getFrameBufferAt(int index)
{
	if (index < 0 || index >= subItems.size()) return nullptr;
	if (subItems[index]->isSubTexture() && index > 0)
	{
		return subItems[0]->getFrameBuffer(subItems[index]->textureName->getValueData().toString());
	}

	return subItems[index]->getFrameBuffer();
}

GLuint MediaListItem::getTextureIDAt(int index)
{
	if (index < 0 || index >= subItems.size()) return GLuint();
	if (subItems[index]->isSubTexture() && index > 0)
	{
		return subItems[0]->getTextureID(subItems[index]->textureName->getValueData().toString());
	}

	return subItems[index]->getTextureID();
}

ShaderMedia* MediaListItem::getShaderMediaAt(int index)
{
	if (index < 0 || index >= subItems.size()) return nullptr;
	return subItems[index]->shaderMedia.get();
}

float MediaListItem::getWeightAt(int index, bool useWeightAtStart)
{
	if (index < 0 || index >= subItems.size()) return useWeightAtStart ? weight->floatValue() : weightAtStart;
	return useWeightAtStart ? subItems[index]->weight : subItems[index]->weightAtStart;
}

Array<float> MediaListItem::getSubTransitionTimes(float defaultTime)
{
	Array<float> result;
	for (auto& s : subItems)
	{
		if (s->transitionTimeOverride->enabled)
			result.add(s->transitionTimeOverride->floatValue());
		else
			result.add(transitionTime->enabled ? transitionTime->floatValue() : defaultTime);
	}
	return result;
}

float MediaListItem::getReferenceLength()
{
	AutoNextBehavior an = (AutoNextBehavior)autoNextBehavior->getValueDataAsEnum<int>();

	float minLength = INT32_MAX;
	float maxLength = 0.f;
	for (auto& s : subItems)
	{
		Media* m = s->media;
		if (m == nullptr) continue;
		float l = m->getMediaLength();
		if (l <= 0) continue;
		if (l < minLength) minLength = l;
		if (l > maxLength) maxLength = l;
	}

	return an == AUTO_NEXT_MEDIA_FINISH_FIRST ? minLength : maxLength;
}



void MediaListItem::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == transitionTime)
	{
		for (auto& s : subItems)
		{
			s->transitionTimeOverride->setRange(0, transitionTime->floatValue());
		}
	}
	else if (p == state)
	{
		TransitionState ts = state->getValueDataAsEnum<TransitionState>();

		for (auto& s : subItems)
		{
			if (ts != IDLE && s->media != nullptr)
			{
				s->registerUseMedia(MEDIALISTITEM_MEDIA_ID, s->media);
			}
			else
			{
				s->unregisterUseMedia(MEDIALISTITEM_MEDIA_ID);
			}

			if (s->shaderMedia->enabled->boolValue())
			{
				if (ts == LOADING)
				{
					s->registerUseMedia(MEDIALISTITEM_TRANSITION_ID, s->shaderMedia.get());
				}
				else {
					s->unregisterUseMedia(MEDIALISTITEM_TRANSITION_ID);
				}
			}

			Media* m = s->media;
			if (m == nullptr) continue;
			if (ts == LOADING && autoPlay->boolValue())
				m->handleEnter(0, true);
			else if (ts == IDLE && autoStop->boolValue())
				m->handleStop();
		}
	}
}

void MediaListItem::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	BaseItem::onControllableFeedbackUpdateInternal(cc, c);


	if (MediaListSubItem* subItem = dynamic_cast<MediaListSubItem*>(cc))
	{
		if (c == subItem->type)
		{
			if (subItem->isSubTexture())
			{
				subItem->updateTextureNameOptions(subItems[0]->media);
			}
		}
	}
}



bool MediaListItem::isLoading() const {
	return state->getValueDataAsEnum<TransitionState>() == LOADING;
}

bool MediaListItem::isUnloading() const {
	return state->getValueDataAsEnum<TransitionState>() == UNLOADING;
}

void MediaListItem::newMessage(const MediaListSubItem::MediaListSubItemEvent& event)
{
	if (isBeingDestroyed || (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing)) return;

	if (event.type == MediaListSubItem::MediaListSubItemEvent::SUBMEDIA_FINISHED)
	{
		TransitionState ts = state->getValueDataAsEnum<TransitionState>();
		AutoNextBehavior an = (AutoNextBehavior)autoNextBehavior->getValueDataAsEnum<int>();
		if (ts == RUNNING && an == AUTO_NEXT_MEDIA_FINISH_FIRST && autoNextTime->floatValue() == 0)
		{
			listItemNotifier.addMessage(new MediaListItemEvent(MediaListItemEvent::AUTO_NEXT, this));
		}
	}
	else if (event.type == MediaListSubItem::MediaListSubItemEvent::SELECTION_CHANGED)
	{
		listItemNotifier.addMessage(new MediaListItemEvent(MediaListItemEvent::SELECTION_CHANGED, this));
	}
}

var MediaListItem::getJSONData(bool includeNonOverriden)
{
	var data = BaseItem::getJSONData(includeNonOverriden);
	var subItemsData = var();
	for (auto& subItem : subItems)
	{
		subItemsData.append(subItem->getJSONData(includeNonOverriden));
	}

	data.getDynamicObject()->setProperty("subItems", subItemsData);

	return data;
}

void MediaListItem::loadJSONDataItemInternal(var data)
{
	BaseItem::loadJSONDataItemInternal(data);
	if (data.hasProperty("subItems"))
	{
		var subItemsData = data["subItems"];
		if (subItemsData.isArray())
		{
			setNumLayers((int)subItemsData.size());
			for (int i = 0; i < subItemsData.size(); i++)
			{
				subItems[i]->loadJSONData(subItemsData[i]);
			}
		}
	}
}