/*
  ==============================================================================

	MediaLayer.cpp
	Created: 21 Dec 2023 10:39:51am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaLayer.h"

MediaLayer::MediaLayer(Sequence* s, var params) :
	SequenceLayer(s, "Media"),
	blockManager(this),
	positionningCC("Positionning")
{
	saveAndLoadRecursiveData = true;

	backgroundColor = addColorParameter("Background Color", "Background Color for this layer", Colours::transparentBlack);

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
	blendFunctionSourceFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	blendFunctionDestinationFactor = addEnumParameter("Blend destination factor", "");
	blendFunctionDestinationFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	blendFunctionSourceFactor->setDefaultValue("GL_SRC_ALPHA");
	blendFunctionDestinationFactor->setDefaultValue("GL_ONE_MINUS_SRC_ALPHA");
	blendFunctionSourceFactor->setControllableFeedbackOnly(true);
	blendFunctionDestinationFactor->setControllableFeedbackOnly(true);


	xParam = positionningCC.addIntParameter("X", "X Position", 0);
	yParam = positionningCC.addIntParameter("Y", "Y Position", 0);
	widthParam = positionningCC.addIntParameter("Width", "Width", 100);
	heightParam = positionningCC.addIntParameter("Height", "Height", 100);
	positionningCC.enabled->setValue(false);
	addChildControllableContainer(&positionningCC);

	addChildControllableContainer(&blockManager);
}

MediaLayer::~MediaLayer()
{
}

void MediaLayer::initFrameBuffer(int width, int height)
{
	if (frameBuffer.isValid()) frameBuffer.release();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, width, height);
	widthParam->setDefaultValue(width, !widthParam->isOverriden);
	heightParam->setDefaultValue(height, !heightParam->isOverriden);
}

bool MediaLayer::renderFrameBuffer(int width, int height)
{
	float time = sequence->currentTime->floatValue();
	Array<LayerBlock*> blocks = blockManager.getBlocksAtTime(time, false);

	Array<MediaClip*> clipsToProcess;

	Array<ClipTransition*> transitions;
	for (auto& b : blocks)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			if (clip->media == nullptr) continue;
			clipsToProcess.add(clip);

			if (auto t = dynamic_cast<ClipTransition*>(clip))
			{
				transitions.add(t);
			}
		}
	}

	if (!transitions.isEmpty())
	{
		clipsToProcess.clear();
		clipsToProcess.add(transitions.getLast());
	}


	//if (clipsToProcess.isEmpty()) return false;

	if (frameBuffer.getWidth() != width || frameBuffer.getHeight() != height) initFrameBuffer(width, height);

	frameBuffer.makeCurrentRenderingTarget();

	Colour c = backgroundColor->getColor();

	if (!positionningCC.enabled->boolValue()) glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);

	if (positionningCC.enabled->boolValue())
	{
		glClearColor(0, 0, 0, 0);
		glColor4f(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
		Draw2DRect(xParam->intValue(), yParam->intValue(), widthParam->intValue(), heightParam->intValue());
	}

	int index = 0;
	for (auto& clip : clipsToProcess)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, clip->media->frameBuffer.getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//draw full quad

		double fadeMultiplier = clip->getFadeMultiplier();
		glColor4f(1, 1, 1, fadeMultiplier);


		if (positionningCC.enabled->boolValue())
		{
			Draw2DTexRect(xParam->intValue(), yParam->intValue(), widthParam->intValue(), heightParam->intValue());
		}
		else
		{
			Draw2DTexRect(0, 0, width, height);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		index++;
	}

	frameBuffer.releaseAsRenderingTarget();

	return true;
}

void MediaLayer::renderGL(int depth)
{
	glBlendFunc((GLenum)(int)blendFunctionSourceFactor->getValueData(), (GLenum)(int)blendFunctionDestinationFactor->getValueData());
	//BlendMode bm = blendMode->getValueDataAsEnum<BlendMode>();

	//switch (bm)
	//{
	//case Alpha:
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	break;

	//case Add:
	//	glBlendFunc(GL_ONE, GL_ONE);
	//	break;

	//case Multiply:
	//	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	//	break;
	//}

	glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glColor4f(1, 1, 1, 1);
	Draw2DTexRectDepth(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight(), depth);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void MediaLayer::sequenceCurrentTimeChanged(Sequence* s, float prevTime, bool evaluateSkippedData)
{
	Array<LayerBlock*> activeBlocks = blockManager.getBlocksAtTime(s->currentTime->floatValue());

	double t = s->currentTime->doubleValue();

	for (auto& b : blockManager.items)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			clip->isActive->setValue(activeBlocks.contains(clip));
			clip->setTime(t, s->isSeeking);
		}
	}
}

void MediaLayer::sequencePlayStateChanged(Sequence* s)
{
	for (auto& b : blockManager.items)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			clip->setIsPlaying(s->isPlaying->boolValue());
		}
	}
}

void MediaLayer::onContainerParameterChangedInternal(Parameter* p)
{
	SequenceLayer::onContainerParameterChangedInternal(p);

	if (p == widthParam || p == heightParam)
	{
		Array<OwnedMediaClip*> clips = blockManager.getItemsWithType<OwnedMediaClip>();
		for (auto& c : clips)
		{
			if (p == widthParam) c->media->width->setValue(widthParam->intValue());
			else c->media->height->setValue(heightParam->intValue());
		}
	}
	else if (p == blendFunction) {
		BlendPreset preset = blendFunction->getValueDataAsEnum<BlendPreset>();
		if (preset == CUSTOM) {
			blendFunctionSourceFactor->setControllableFeedbackOnly(false);
			blendFunctionDestinationFactor->setControllableFeedbackOnly(false);
		}
		else
		{
			blendFunctionSourceFactor->setControllableFeedbackOnly(true);
			blendFunctionDestinationFactor->setControllableFeedbackOnly(true);
			switch (preset)
			{
			case STANDARD:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case ADDITION:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case MULTIPLICATION:
				blendFunctionSourceFactor->setValueWithData((int)GL_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ZERO);
				break;
			case SCREEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case DARKEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case PREMULTALPHA:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case LIGHTEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORADD:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_DST_COLOR);
				break;
			case COLORSCREEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case BLUR:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERTCOLOR:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case SUBSTRACT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ZERO);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORDIFF:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
				break;
			case INVERTMULT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
				break;

			}
		}
	}
}

void MediaLayer::getSnapTimes(Array<float>* arrayToFill)
{
	blockManager.getSnapTimes(arrayToFill);
}


SequenceLayerTimeline* MediaLayer::getTimelineUI()
{
	return new MediaLayerTimeline(this);
}
