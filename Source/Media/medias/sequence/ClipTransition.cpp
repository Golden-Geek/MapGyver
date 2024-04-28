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
	mediaOutParam(nullptr)
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
}

void ClipTransition::setInOutMedia(MediaClip* in, MediaClip* out)
{
	inMedia = in;
	outMedia = out;

	mediaInParam->setValueFromTarget(inMedia->media);
	mediaOutParam->setValueFromTarget(outMedia->media);
}

void ClipTransition::setTime(double t, bool seekMode)
{
	MediaClip::setTime(t, seekMode);
	progressParam->setValue(fadeCurve.getValueAtPosition((t - time->floatValue()) / getTotalLength()));
}

void ClipTransition::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	MediaClip::onControllableFeedbackUpdateInternal(cc, c);
	if (c == mediaInParam) inMedia = ControllableUtil::findParentAs<MediaClip>(mediaInParam->targetContainer.get());
	else if (c == mediaOutParam) outMedia = ControllableUtil::findParentAs<MediaClip>(mediaOutParam->targetContainer.get());
}

void ClipTransition::loadJSONDataItemInternal(var data)
{
	MediaClip::loadJSONDataItemInternal(data);
}