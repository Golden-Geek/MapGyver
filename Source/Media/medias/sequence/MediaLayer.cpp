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
	blockManager(this)
{
	saveAndLoadRecursiveData = true;

	backgroundColor = addColorParameter("Background Color", "Background Color for this layer", Colours::transparentBlack);

	blendMode = addEnumParameter("Blend Mode", "Blend Mode for this layer");
	blendMode->addOption("Add", Add)->addOption("Alpha", Alpha)->addOption("Multiply", Multiply);

	addChildControllableContainer(&blockManager);
}

MediaLayer::~MediaLayer()
{
}

void MediaLayer::initFrameBuffer(int width, int height)
{
	if (frameBuffer.isValid()) frameBuffer.release();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, width, height);
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

	BlendMode bm = blendMode->getValueDataAsEnum<BlendMode>();
	Colour c = backgroundColor->getColor();

	if (bm == Multiply) glClearColor(1, 1, 1, 1);
	else glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);

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

		Draw2DTexRect(0, 0, width, height);

		glBindTexture(GL_TEXTURE_2D, 0);

		index++;
	}

	frameBuffer.releaseAsRenderingTarget();

	return true;
}

void MediaLayer::renderGL(int depth)
{

	BlendMode bm = blendMode->getValueDataAsEnum<BlendMode>();

	switch (bm)
	{
	case Alpha:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case Add:
		glBlendFunc(GL_ONE, GL_ONE);
		break;

	case Multiply:
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	}

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


SequenceLayerTimeline* MediaLayer::getTimelineUI()
{
	return new MediaLayerTimeline(this);
}
