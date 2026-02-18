/*
  ==============================================================================

	ClipTransition.cpp
	Created: 27 Apr 2024 12:48:07pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

ClipTransition::ClipTransition(var params) :
	MediaClip(getTypeString(), params),
	mediaInParam(nullptr),
	mediaOutParam(nullptr),
	inClip(nullptr),
	outClip(nullptr)
{
	var sParams(new DynamicObject());
	//sParams.getDynamicObject()->setProperty("manualRender", true);
	shaderMedia.reset(new ShaderMedia(sParams));

	progressParam = shaderMedia->mediaParams.addFloatParameter("progression", "progression", 0, 0, 1);
	progressParam->isRemovableByUser = false;
	progressParam->setControllableFeedbackOnly(true);

	coreLength->clearRange();
	coreLength->setControllableFeedbackOnly(true);

	loopLength->setControllableFeedbackOnly(true);
	loopLength->hideInEditor = true;

	shaderMedia->backgroundColor->setDefaultValue(Colours::transparentBlack, true);

	addChildControllableContainer(shaderMedia.get());
	setMedia(shaderMedia.get());

	clipInParam = addTargetParameter("Clip In", "Clip In");
	clipInParam->targetType = TargetParameter::CONTAINER;

	clipOutParam = addTargetParameter("Clip Out", "Clip Out");
	clipOutParam->targetType = TargetParameter::CONTAINER;

	mediaInParam = shaderMedia->sourceMedias.addTargetParameter("Media In", "Media In");
	mediaInParam->targetType = TargetParameter::CONTAINER;

	mediaOutParam = shaderMedia->sourceMedias.addTargetParameter("Media Out", "Media Out");
	mediaOutParam->targetType = TargetParameter::CONTAINER;
}

ClipTransition::~ClipTransition()
{
	setInClip(nullptr);
	setOutClip(nullptr);
}


void ClipTransition::clearItem()
{
	shaderMedia->clearItem();
	MediaClip::clearItem();
}

void ClipTransition::setInOutClips(MediaClip* in, MediaClip* out)
{
	clipInParam->setValueFromTarget(in);
	clipOutParam->setValueFromTarget(out);

	computeTimes(nullptr);
}

void ClipTransition::setTime(double t, bool seekMode)
{
	MediaClip::setTime(t, seekMode);
	progressParam->setValue(fadeCurve.getValueAtPosition((t - time->floatValue()) / getTotalLength()));
}

void ClipTransition::setInClip(MediaClip* in)
{
	if (in == inClip) return;

	if (inClip != nullptr)
	{
		if (!inClip->isClearing)
		{
			inClip->setOutTransition(nullptr);
			inClip->time->removeParameterListener(this);
			inClip->coreLength->removeParameterListener(this);
			inClip->loopLength->removeParameterListener(this);
			if (auto r = dynamic_cast<ReferenceMediaClip*>(inClip)) r->mediaTarget->removeParameterListener(this);
			mediaInParam->setValueFromTarget((ControllableContainer*)nullptr);
		}

	}
	inClip = in;
	if (inClip != nullptr)
	{
		inClip->setOutTransition(this);
		inClip->time->addParameterListener(this);
		inClip->coreLength->addParameterListener(this);
		inClip->loopLength->addParameterListener(this);
		if (auto r = dynamic_cast<ReferenceMediaClip*>(inClip)) r->mediaTarget->addParameterListener(this);
		mediaInParam->setValueFromTarget(inClip->media);

		computeTimes(inClip);
	}

}

void ClipTransition::setOutClip(MediaClip* out)
{
	if (out == outClip) return;

	if (outClip != nullptr)
	{
		if (!outClip->isClearing)
		{

			outClip->setInTransition(nullptr);
			outClip->time->removeParameterListener(this);
			outClip->coreLength->removeParameterListener(this);
			outClip->loopLength->removeParameterListener(this);
			if (auto r = dynamic_cast<ReferenceMediaClip*>(outClip)) r->mediaTarget->removeParameterListener(this);
			mediaOutParam->setValueFromTarget((ControllableContainer*)nullptr);
		}
	}
	outClip = out;
	if (outClip != nullptr)
	{
		outClip->setInTransition(this);
		outClip->time->addParameterListener(this);
		outClip->coreLength->addParameterListener(this);
		outClip->loopLength->addParameterListener(this);
		if (auto r = dynamic_cast<ReferenceMediaClip*>(outClip)) r->mediaTarget->addParameterListener(this);
		mediaOutParam->setValueFromTarget(outClip->media);
		computeTimes(outClip);
	}
}

void ClipTransition::computeTimes(MediaClip* origin)
{
	if (isCurrentlyLoadingData) return;

	if (inClip != nullptr && outClip != nullptr)
	{
		float inTime = inClip->getEndTime();
		float outTime = outClip->time->floatValue();
		float minTime = jmin(inTime, outTime);
		float maxTime = jmax(inTime, outTime);

		time->setValue(minTime);
		setCoreLength(maxTime - minTime);

		if (origin != inClip) inClip->dispatchTransitionChanged();
		if (origin != outClip) outClip->dispatchTransitionChanged();
	}
}

void ClipTransition::onContainerParameterChangedInternal(Parameter* p)
{
	MediaClip::onContainerParameterChangedInternal(p);
	if (p == clipInParam) setInClip(clipInParam->getTargetContainerAs<MediaClip>());
	else if (p == clipOutParam) setOutClip(clipOutParam->getTargetContainerAs<MediaClip>());
}


void ClipTransition::onExternalParameterValueChanged(Parameter* p)
{
	MediaClip::onExternalParameterValueChanged(p);
	if (p->parentContainer == inClip)
	{
		if (auto r = dynamic_cast<ReferenceMediaClip*>(inClip)) mediaInParam->setValueFromTarget(r->media);
		computeTimes(inClip);
	}
	else if (p->parentContainer == outClip)
	{
		if (auto r = dynamic_cast<ReferenceMediaClip*>(outClip)) mediaOutParam->setValueFromTarget(r->media);
		computeTimes(outClip);
	}
}

void ClipTransition::loadJSONDataItemInternal(var data)
{
	MediaClip::loadJSONDataItemInternal(data);
}