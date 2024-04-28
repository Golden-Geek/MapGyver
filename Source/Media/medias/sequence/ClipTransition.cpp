/*
  ==============================================================================

	ClipTransition.cpp
	Created: 27 Apr 2024 12:48:07pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "ClipTransition.h"

ClipTransition::ClipTransition(var params) :
	MediaClip(getTypeString(), params),
	mediaInParam(nullptr),
	mediaOutParam(nullptr),
	inClip(nullptr),
	outClip(nullptr)
{
	progressParam = shaderMedia.mediaParams.addFloatParameter("progression", "progression", 0, 0, 1);
	progressParam->isRemovableByUser = false;
	progressParam->setControllableFeedbackOnly(true);
	shaderMedia.backgroundColor->setDefaultValue(Colours::transparentBlack, true);

	addChildControllableContainer(&shaderMedia);
	setMedia(&shaderMedia);

	mediaInParam = shaderMedia.sourceMedias.addTargetParameter("Media In", "Media In", MediaManager::getInstance());
	mediaInParam->targetType = TargetParameter::CONTAINER;

	mediaOutParam = shaderMedia.sourceMedias.addTargetParameter("Media Out", "Media Out", MediaManager::getInstance());
	mediaOutParam->targetType = TargetParameter::CONTAINER;
}

ClipTransition::~ClipTransition()
{
	setInClip(false);
	setOutClip(false);
}

void ClipTransition::setInOutMedia(MediaClip* in, MediaClip* out)
{
	mediaInParam->setValueFromTarget(inClip->media);
	mediaOutParam->setValueFromTarget(outClip->media);
}

void ClipTransition::setTime(double t, bool seekMode)
{
	MediaClip::setTime(t, seekMode);
	progressParam->setValue(fadeCurve.getValueAtPosition((t - time->floatValue()) / getTotalLength()));
}

void ClipTransition::setInClip(MediaClip* in)
{
	if (inClip != nullptr) inClip->setOutTransition(nullptr);
	inClip = in;
	if(inClip != nullptr) inClip->setOutTransition(this);
}

void ClipTransition::setOutClip(MediaClip* out)
{
	if (outClip != nullptr) outClip->setInTransition(nullptr);
	outClip = out;
	if (outClip != nullptr) outClip->setInTransition(this);
}

void ClipTransition::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	MediaClip::onControllableFeedbackUpdateInternal(cc, c);
	if (c == mediaInParam) setInClip(ControllableUtil::findParentAs<MediaClip>(mediaInParam->targetContainer.get()));
	else if (c == mediaOutParam) setOutClip(ControllableUtil::findParentAs<MediaClip>(mediaOutParam->targetContainer.get()));
}

void ClipTransition::loadJSONDataItemInternal(var data)
{
	MediaClip::loadJSONDataItemInternal(data);
}