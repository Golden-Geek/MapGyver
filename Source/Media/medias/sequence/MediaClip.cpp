/*
  ==============================================================================

	MediaClip.cpp
	Created: 21 Dec 2023 10:40:39am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaClip::MediaClip(var params) :
	LayerBlock(getTypeString()),
	media(nullptr),
	fadeCurve("Fade Curve"),
	settingLengthFromMethod(false),
	mediaClipNotifier(5)
{
	saveAndLoadRecursiveData = true;

	mediaTarget = addTargetParameter("Media", "Media to use for this clip", MediaManager::getInstance());
	mediaTarget->targetType = TargetParameter::CONTAINER;
	mediaTarget->maxDefaultSearchLevel = 0;

	fadeIn = addFloatParameter("Fade In", "Fade in time", 0, 0, getTotalLength(), false);
	fadeIn->defaultUI = FloatParameter::TIME;
	fadeIn->canBeDisabledByUser = true;
	fadeOut = addFloatParameter("Fade Out", "Fade out time", 0, 0, getTotalLength(), false);
	fadeOut->defaultUI = FloatParameter::TIME;
	fadeOut->canBeDisabledByUser = true;


	fadeCurve.addKey(0, 0);
	fadeCurve.addKey(1, 1);
	addChildControllableContainer(&fadeCurve);
}

MediaClip::~MediaClip()
{
}

void MediaClip::clearItem()
{
	LayerBlock::clearItem();
	setMedia(nullptr);
}

void MediaClip::setMedia(Media* m)
{
	if (m == media) return;
	if (media != nullptr)
	{
		unregisterUseMedia(CLIP_MEDIA_ID);
	}

	media = m;

	if (media != nullptr)
	{
		registerUseMedia(CLIP_MEDIA_ID, media);
	}
}

void MediaClip::onContainerParameterChangedInternal(Parameter* p)
{
	LayerBlock::onContainerParameterChangedInternal(p);

	if (p == mediaTarget)
	{
		setMedia(mediaTarget->getTargetContainerAs<Media>());
	}
	if (p == coreLength || p == loopLength)
	{
		fadeIn->setRange(0, getTotalLength());
		fadeOut->setRange(0, getTotalLength());

		if (p == coreLength && !settingLengthFromMethod) //force refresh automation
		{
			setCoreLength(coreLength->floatValue());
		}
	}
	else if (p == fadeIn || p == fadeOut)
	{
		mediaClipListeners.call(&MediaClipListener::mediaClipFadesChanged, this);
		mediaClipNotifier.addMessage(new MediaClipEvent(MediaClipEvent::FADES_CHANGED, this));
	}
}

void MediaClip::controllableStateChanged(Controllable* c)
{
	LayerBlock::controllableStateChanged(c);
	if (c == fadeIn || c == fadeOut)
	{
		mediaClipListeners.call(&MediaClipListener::mediaClipFadesChanged, this);
		mediaClipNotifier.addMessage(new MediaClipEvent(MediaClipEvent::FADES_CHANGED, this));
	}
}

void MediaClip::setCoreLength(float value, bool stretch, bool stickToCoreEnd)
{
	settingLengthFromMethod = true;
	LayerBlock::setCoreLength(value, stretch, stickToCoreEnd);

	Array<WeakReference<Parameter>> params = media->mediaParams.getAllParameters(true);
	for (auto& pa : params)
	{
		if (pa->automation == nullptr) continue;
		pa->automation->setAllowKeysOutside(true);
		pa->automation->setLength(coreLength->floatValue(), stretch, stickToCoreEnd);

	}
	settingLengthFromMethod = false;
}

float MediaClip::getFadeMultiplier(float absoluteTime)
{
	double relTimeTotal = absoluteTime - time->floatValue();

	float result = 1;
	if (fadeIn->floatValue() > 0) result *= fadeCurve.getValueAtPosition(jmin<double>(relTimeTotal / fadeIn->floatValue(), 1.f));
	if (fadeOut->floatValue() > 0) result *= fadeCurve.getValueAtPosition(jmin<double>((getTotalLength() - relTimeTotal) / fadeOut->floatValue(), 1.f));
	return result;
}

bool MediaClip::isUsingMedia(Media* m)
{
	return enabled->boolValue() && media == m;
}
