/*
  ==============================================================================

	MediaLayer.cpp
	Created: 21 Dec 2023 10:39:51am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaLayer::MediaLayer(Sequence* s, var params) :
	SequenceLayer(s, "Media"),
	blockManager(this),
	positionningCC("Positionning")
{
	saveAndLoadRecursiveData = true;

	backgroundColor = addColorParameter("Background Color", "Background Color for this layer", Colours::black);

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


	transitionBlendFunction = addEnumParameter("Transition Blend function", "");
	transitionBlendFunction
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

	transitionBlendFunctionSourceFactor = addEnumParameter("Transition Blend source factor", "");
	transitionBlendFunctionSourceFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	transitionBlendFunctionDestinationFactor = addEnumParameter("Transition Blend destination factor", "");
	transitionBlendFunctionDestinationFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	transitionBlendFunction->setDefaultValue("Lighten");
	//transitionBlendFunctionSourceFactor->setDefaultValue("GL_SRC_ALPHA");
	//transitionBlendFunctionDestinationFactor->setDefaultValue("GL_ONE_MINUS_SRC_ALPHA");
	transitionBlendFunctionSourceFactor->setControllableFeedbackOnly(true);
	transitionBlendFunctionDestinationFactor->setControllableFeedbackOnly(true);


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

			if (clip->activationChanged)
			{
				if (Media* m = clip->media) m->renderOpenGLMedia(true);
				clip->activationChanged = false;
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


	for (auto& clip : clipsToProcess)
	{
		clip->media->renderOpenGLMedia(false);
	}

	frameBuffer.makeCurrentRenderingTarget();
	Init2DViewport(frameBuffer.getWidth(), frameBuffer.getHeight());

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
		glBlendFunc((GLenum)(int)transitionBlendFunctionSourceFactor->getValueData(), (GLenum)(int)transitionBlendFunctionDestinationFactor->getValueData());
		glBindTexture(GL_TEXTURE_2D, clip->media->frameBuffer.getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//draw full quad

		double fadeMultiplier = clip->getFadeMultiplier();
		glColor4f(1, 1, 1, fadeMultiplier);

		Rectangle<int> layerBounds = Rectangle<int>(0, 0, width, height);
		if (positionningCC.enabled->boolValue())
			layerBounds.setBounds(xParam->intValue(), yParam->intValue(), widthParam->intValue(), heightParam->intValue());

		Rectangle<int> clipBounds = clip->media->getMediaRect(layerBounds);

		Draw2DTexRect(clipBounds.getX(), clipBounds.getY(), clipBounds.getWidth(), clipBounds.getHeight());

		glBindTexture(GL_TEXTURE_2D, 0);

		index++;
	}

	frameBuffer.releaseAsRenderingTarget();

	return true;
}

void MediaLayer::renderGL(int depth)
{
	glBlendFunc((GLenum)(int)blendFunctionSourceFactor->getValueData(), (GLenum)(int)blendFunctionDestinationFactor->getValueData());
	//BlendMode bm = blendMode->getValueData<BlendMode>();

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
	double t = s->currentTime->doubleValue();
	//Array<LayerBlock*> activeBlocks = blockManager.getBlocksInRange(t, nextFrameTime);// getBlocksInRange(s->currentTime->floatValue(), s->currentTime->floatValue() + .01f, false);

	for (auto& b : blockManager.items)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			clip->setTime(t, s->isSeeking || !s->isPlaying->boolValue());

			bool hasPreTransition = clip->inTransition != nullptr;
			bool hasPostTransition = clip->outTransition != nullptr;
			float preTimeRef = hasPreTransition ? clip->inTransition->time->floatValue() : clip->time->floatValue();
			float postTimeRef = hasPostTransition ? clip->outTransition->getEndTime() : clip->getEndTime();

			float enterTime = preTimeRef - clip->preStart->floatValue();
			float leaveTime = postTimeRef + clip->postEnd->floatValue();

			bool isActive = t >= enterTime && t <= leaveTime;

			if (isActive != clip->isActive->boolValue())
			{
				clip->isActive->setValue(isActive);
			}

			clip->setTime(t, s->isSeeking || !s->isPlaying->boolValue());
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
	else if (p == transitionBlendFunction) {
		BlendPreset preset = transitionBlendFunction->getValueDataAsEnum<BlendPreset>();
		if (preset == CUSTOM) {
			transitionBlendFunctionSourceFactor->setControllableFeedbackOnly(false);
			transitionBlendFunctionDestinationFactor->setControllableFeedbackOnly(false);
		}
		else
		{
			transitionBlendFunctionSourceFactor->setControllableFeedbackOnly(true);
			transitionBlendFunctionDestinationFactor->setControllableFeedbackOnly(true);
			switch (preset)
			{
			case STANDARD:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case ADDITION:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case MULTIPLICATION:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_DST_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ZERO);
				break;
			case SCREEN:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case DARKEN:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_ALPHA);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case PREMULTALPHA:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case LIGHTEN:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERT:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORADD:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_SRC_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_DST_COLOR);
				break;
			case COLORSCREEN:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case BLUR:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERTCOLOR:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case SUBSTRACT:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ZERO);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORDIFF:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
				break;
			case INVERTMULT:
				transitionBlendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				transitionBlendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
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
