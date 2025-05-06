/*
  ==============================================================================

	MediaClip.cpp
	Created: 21 Dec 2023 10:40:39am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaClip.h"

MediaClip::MediaClip(const String& name, var params) :
	LayerBlock(name),
	media(nullptr),
	relativeTime(-1),
	isPlaying(false),
	fadeCurve("Fade Curve"),
	settingLengthFromMethod(false),
	inTransition(nullptr),
	outTransition(nullptr),
	activationChanged(false),
	mediaClipNotifier(5)
{
	saveAndLoadRecursiveData = true;


	fadeIn = addFloatParameter("Fade In", "Fade in time", 0, 0, getTotalLength(), false);
	fadeIn->defaultUI = FloatParameter::TIME;
	fadeIn->canBeDisabledByUser = true;
	fadeOut = addFloatParameter("Fade Out", "Fade out time", 0, 0, getTotalLength(), false);
	fadeOut->defaultUI = FloatParameter::TIME;
	fadeOut->canBeDisabledByUser = true;

	preStart = addFloatParameter("Pre Start", "Pre start time", 0, 0, 10);
	preStart->defaultUI = FloatParameter::TIME;
	postEnd = addFloatParameter("Post End", "Post end time", 0, 0, 10);
	postEnd->defaultUI = FloatParameter::TIME;


	fadeCurve.addKey(0, 0);
	fadeCurve.addKey(1, 1);
	addChildControllableContainer(&fadeCurve);
}

MediaClip::~MediaClip()
{
}

void MediaClip::clearItem()
{
	clearTarget();

	LayerBlock::clearItem();
	setMedia(nullptr);
	if (media != nullptr && !mediaRef.wasObjectDeleted()) unregisterUseMedia(CLIP_MEDIA_ID);
}

void MediaClip::setMedia(Media* m)
{
	if (m == media) return;
	if (media != nullptr && !mediaRef.wasObjectDeleted() && !media->isClearing)
	{
		media->removeAsyncMediaListener(this);
		media->handleExit();
		unregisterUseMedia(CLIP_MEDIA_ID);
	}

	media = m;
	mediaRef = media;

	if (media != nullptr)
	{
		media->addAsyncMediaListener(this);
		registerUseMedia(CLIP_MEDIA_ID, media);
		media->handleEnter(relativeTime);
		if (isPlaying) media->handleStart();
	}

	mediaClipListeners.call(&MediaClipListener::mediaChanged, this);
	mediaClipNotifier.addMessage(new MediaClipEvent(MediaClipEvent::MEDIA_CHANGED, this));
}

void MediaClip::setTime(double t, bool seekMode)
{
	relativeTime = getRelativeTime(t, true);
	if (!enabled->boolValue() || !isActive->boolValue()) return;
	if (media != nullptr) media->setCustomTime(relativeTime, seekMode);
}

void MediaClip::setInTransition(ClipTransition* t)
{
	if (isClearing) return;
	inTransition = t;
	dispatchTransitionChanged();

}

void MediaClip::setOutTransition(ClipTransition* t)
{
	if (isClearing) return;
	outTransition = t;
	dispatchTransitionChanged();
}

void MediaClip::dispatchTransitionChanged()
{
	mediaClipNotifier.addMessage(new MediaClipEvent(MediaClipEvent::TRANSITIONS_CHANGED, this));
}

void MediaClip::onContainerParameterChangedInternal(Parameter* p)
{
	LayerBlock::onContainerParameterChangedInternal(p);

	if (p == isActive || p == enabled)
	{
		bool active = isActive->boolValue() && enabled->boolValue();

		if (media != nullptr)
		{
			activationChanged = true;
		
			if (active)
			{
				media->handleEnter(relativeTime, isPlaying);
				media->updateBeingUsed();
			}
			else
			{
				media->handleExit();
				media->updateBeingUsed();
			}
		}
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

void MediaClip::setIsPlaying(bool playing)
{
	if (playing == isPlaying) return;

	isPlaying = playing;

	if (media != nullptr && isActive->boolValue() && enabled->boolValue())
	{
		if (isPlaying) media->handleStart();
		else media->handleStop();
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

	if (media != nullptr)
	{
		Array<WeakReference<Parameter>> params = media->mediaParams.getAllParameters(true);
		for (auto& pa : params)
		{
			if (pa->automation == nullptr) continue;
			pa->automation->setAllowKeysOutside(true);
			pa->automation->setLength(coreLength->floatValue(), stretch, stickToCoreEnd);

		}

	}

	settingLengthFromMethod = false;
}

float MediaClip::getFadeMultiplier()
{
	//double relTimeTotal = absoluteTime - time->floatValue();

	float result = 1;
	if (fadeIn->floatValue() > 0 && inTransition == nullptr) result *= fadeCurve.getValueAtPosition(jmin<double>(relativeTime / fadeIn->floatValue(), 1.f));
	if (fadeOut->floatValue() > 0 && outTransition == nullptr) result *= fadeCurve.getValueAtPosition(jmin<double>((getTotalLength() - relativeTime) / fadeOut->floatValue(), 1.f));
	return result;
}

bool MediaClip::isUsingMedia(Media* m)
{
	return enabled->boolValue() && isActive->boolValue() && media == m;
}

void MediaClip::newMessage(const Media::MediaEvent& e)
{
	switch (e.type)
	{
	case Media::MediaEvent::PREVIEW_CHANGED:
		mediaClipNotifier.addMessage(new MediaClipEvent(MediaClipEvent::PREVIEW_CHANGED, this));
		break;

	default:
		break;
	}
}

ReferenceMediaClip::ReferenceMediaClip(var params) :
	MediaClip(getTypeString(), params)
{

	mediaTarget = addTargetParameter("Media", "Media to use for this clip", MediaManager::getInstance());
	mediaTarget->targetType = TargetParameter::CONTAINER;
	mediaTarget->maxDefaultSearchLevel = 0;
}

ReferenceMediaClip::~ReferenceMediaClip()
{
}

void ReferenceMediaClip::onContainerParameterChangedInternal(Parameter* p)
{
	MediaClip::onContainerParameterChangedInternal(p);

	if (isClearing) return;
	if (p == mediaTarget) setMedia(mediaTarget->getTargetContainerAs<Media>());
}

OwnedMediaClip::OwnedMediaClip(var params) :
	MediaClip(params.getProperty("mediaType", "[notype]").toString(), params),
	ownedMedia(nullptr)
{
	var extraParams(new DynamicObject());
	//extraParams.getDynamicObject()->setProperty("manualRender", true);
	Media* m = MediaManager::getInstance()->factory.createWithExtraParams(params.getProperty("mediaType", "").toString(), extraParams);
	setMedia(m);
}

OwnedMediaClip::OwnedMediaClip(Media* m) :
	MediaClip(m->getTypeString())
{
	setMedia(m);
}

OwnedMediaClip::~OwnedMediaClip()
{
}

void OwnedMediaClip::setMedia(Media* m)
{
	if (m == media) return;

	if (ownedMedia != nullptr)
	{
		ownedMedia->removeAsyncMediaListener(this);
		removeChildControllableContainer(ownedMedia.get());
		ownedMedia.reset();
	}

	MediaClip::setMedia(m);

	if (media != nullptr)
	{
		ownedMedia.reset(media);
		media->addAsyncMediaListener(this);
		addChildControllableContainer(ownedMedia.get());
	}
	else
	{
		ownedMedia.reset();
	}

	
}
