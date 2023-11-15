/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "CompositionLayer.h"
#include "CompositionLayerManager.h"
#include "Definitions/Media/MediaManager.h"

CompositionLayer::CompositionLayer(var params) :
	BaseItem(params.getProperty("name", "CompositionLayer")),
	objectType(params.getProperty("type", "CompositionLayer").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	canBeDisabled = true;

	itemDataType = "CompositionLayer";


	media = addTargetParameter("Media", "Media", MediaManager::getInstance());
	media->maxDefaultSearchLevel = 0;
	media->targetType = TargetParameter::CONTAINER;

	position = addPoint2DParameter("Position", "In pixels");
	size = addPoint2DParameter("Size", "In pixels");
	alpha = addFloatParameter("Alpha", "", 1,0,1);
	rotation = addFloatParameter("Rotation", "", 0,0,360);
}

CompositionLayer::~CompositionLayer()
{
}

void CompositionLayer::onContainerParameterChangedInternal(Parameter* p) {
}

