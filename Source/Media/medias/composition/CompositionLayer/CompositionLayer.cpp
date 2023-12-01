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

	blendFunction = addEnumParameter("Blend function", "");
	blendFunction
		->addOption("Standard Transparency", STANDARD)
		->addOption("Addition", ADDITION)
		->addOption("Multiplication", MULTIPLICATION)
		->addOption("Screen", SCREEN)
		->addOption("Darken", DARKEN)
		->addOption("Premultiplied Alpha", PREMULTALPHA)
		->addOption("Lighten", LIGHTEN)
		->addOption("Inversion", INVERT)
		->addOption("Color Addition", COLORADD)
		->addOption("Color Screen", COLORSCREEN)
		->addOption("Blur Effect", BLUR)
		->addOption("Inverse Color", INVERTCOLOR)
		->addOption("Subtraction", SUBSTRACT)
		->addOption("Color Difference", COLORDIFF)
		->addOption("Inverse Multiplication", INVERTMULT)
		->addOption("Custom", CUSTOM);

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
	blendFunctionSourceFactor->setEnabled(false);
	blendFunctionDestinationFactor->setEnabled(false);



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
	else if (p == blendFunction) {
		blendPreset preset = blendFunction->getValueDataAsEnum<blendPreset>();
		if (preset == CUSTOM) {
			blendFunctionSourceFactor->setEnabled(true);
			blendFunctionDestinationFactor->setEnabled(true);
		}
		else 
		{
			blendFunctionSourceFactor->setEnabled(false);
			blendFunctionDestinationFactor->setEnabled(false);
			switch (preset)
			{
			case STANDARD:
				blendFunctionSourceFactor->setValueWithData(SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData(ONE_MINUS_SRC_ALPHA);
				break;
			case ADDITION:
				blendFunctionSourceFactor->setValueWithData(ONE);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case MULTIPLICATION:
				blendFunctionSourceFactor->setValueWithData(DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData(ZERO);
				break;
			case SCREEN:
				blendFunctionSourceFactor->setValueWithData(ONE);
				blendFunctionDestinationFactor->setValueWithData(ONE_MINUS_SRC_COLOR);
				break;
			case DARKEN:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_DST_ALPHA);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case PREMULTALPHA:
				blendFunctionSourceFactor->setValueWithData(ONE);
				blendFunctionDestinationFactor->setValueWithData(ONE_MINUS_SRC_ALPHA);
				break;
			case LIGHTEN:
				blendFunctionSourceFactor->setValueWithData(SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case INVERT:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData(ONE_MINUS_SRC_COLOR);
				break;
			case COLORADD:
				blendFunctionSourceFactor->setValueWithData(SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData(DST_COLOR);
				break;
			case COLORSCREEN:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case BLUR:
				blendFunctionSourceFactor->setValueWithData(SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case INVERTCOLOR:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData(ONE);
				break;
			case SUBSTRACT:
				blendFunctionSourceFactor->setValueWithData(ZERO);
				blendFunctionDestinationFactor->setValueWithData(ONE_MINUS_SRC_COLOR);
				break;
			case COLORDIFF:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData(SRC_COLOR);
				break;
			case INVERTMULT:
				blendFunctionSourceFactor->setValueWithData(ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData(SRC_COLOR);
				break;

			}
		}
	}
}

bool CompositionLayer::isUsingMedia(Media* m)
{
	if (!enabled->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
}

