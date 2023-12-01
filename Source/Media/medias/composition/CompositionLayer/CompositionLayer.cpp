/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

#define COMPOSITION_TARGET_MEDIA_ID 0


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
	alpha = addFloatParameter("Alpha", "", 1, 0, 1);
	rotation = addFloatParameter("Rotation", "", 0, 0, 360);

	blendFunctionSourceFactor = addEnumParameter("Blend source factor", "");
	blendFunctionSourceFactor->addOption("GL_ZERO", ZERO)
		->addOption("GL_ONE", ONE)
		->addOption("GL_SRC_ALPHA", SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", ONE_MINUS_DST_COLOR);

	blendFunctionDestinationFactor = addEnumParameter("Blend destination factor", "");
	blendFunctionDestinationFactor->addOption("GL_ZERO", ZERO)
		->addOption("GL_ONE", ONE)
		->addOption("GL_SRC_ALPHA", SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", ONE_MINUS_DST_COLOR);

	blendFunctionSourceFactor->setDefaultValue("GL_SRC_ALPHA");
	blendFunctionDestinationFactor->setDefaultValue("GL_ONE_MINUS_SRC_ALPHA");



}

CompositionLayer::~CompositionLayer()
{
}

void CompositionLayer::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == media)
	{
		if (Media* m = media->getTargetContainerAs<Media>()) registerUseMedia(COMPOSITION_TARGET_MEDIA_ID, m);
		else unregisterUseMedia(COMPOSITION_TARGET_MEDIA_ID);
	}

}

bool CompositionLayer::isUsingMedia(Media* m)
{
	if (!enabled->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
}

