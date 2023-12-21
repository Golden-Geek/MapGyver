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
	blockManager(this)
{
	saveAndLoadRecursiveData = true;

	blendMode = addEnumParameter("Blend Mode", "Blend Mode for this layer");
	blendMode->addOption("Alpha", Alpha)->addOption("Add", Add)->addOption("Multiply", Multiply);

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

void MediaLayer::renderFrameBuffer(int width, int height)
{
	float time = sequence->currentTime->floatValue();
	Array<LayerBlock*> blocks = blockManager.getBlocksAtTime(time, false);

	if (frameBuffer.getWidth() != width || frameBuffer.getHeight() != height) initFrameBuffer(width, height);

	frameBuffer.makeCurrentRenderingTarget();
	glColor4f(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& b : blocks)
	{
		if (MediaClip* clip = dynamic_cast<MediaClip*>(b))
		{
			if (clip->media == nullptr) continue;


			glBindTexture(GL_TEXTURE_2D, clip->media->frameBuffer.getTextureID());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



			//draw full quad

			double fadeMultiplier = clip->getFadeMultiplier();

			glColor4f(1, 1, 1, fadeMultiplier);
			Draw2DTexRect(0, 0, width, height);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	frameBuffer.releaseAsRenderingTarget();
}

void MediaLayer::renderGL()
{
	glEnable(GL_BLEND);

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

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);

	glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glColor3f(1, 1, 1);
	Draw2DTexRect(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
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
