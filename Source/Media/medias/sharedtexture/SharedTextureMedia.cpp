/*
  ==============================================================================

	eSharedTextureMedia.cpp
	Created: 22 Nov 2023 9:45:13pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

BaseSharedTextureMedia::BaseSharedTextureMedia(const String& name, var params) :
	Media(name, params),
	receiver(nullptr),
	shouldReinitFrameBuffer(false)
{
	sharingName = addStringParameter("Sharing name", "Sharing name", "");
	if (!Engine::mainEngine->isLoadingFile) setupReceiver();
}

BaseSharedTextureMedia::~BaseSharedTextureMedia()
{
	if (receiver != nullptr)
	{
		if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->removeReceiver(receiver);
	}

	for (auto& r : extraReceivers)
	{
		if (r != nullptr)
		{
			if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->removeReceiver(r);
		}
	}
}


void BaseSharedTextureMedia::setupReceiver()
{
	if (receiver != nullptr) SharedTextureManager::getInstance()->removeReceiver(receiver);
	receiver = SharedTextureManager::getInstance()->addReceiver(sharingName->stringValue());
	if (receiver != nullptr) receiver->addListener(this);
}

void BaseSharedTextureMedia::setupExtraReceivers(Array<SharedTextureReceiver*> receivers)
{
	for (auto& r : extraReceivers)
	{
		if (r != nullptr)
		{
			r->removeListener(this);
			SharedTextureManager::getInstance()->removeReceiver(r);
		}
	}

	for (auto& r : receivers)
	{
		if (r != nullptr) r->addListener(this);
	}

	extraReceivers = receivers;
	shouldReinitFrameBuffer = true;
}

void BaseSharedTextureMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == sharingName)
	{
		if (!isCurrentlyLoadingData) setupReceiver();
	}
}

void BaseSharedTextureMedia::textureUpdated(SharedTextureReceiver* receiver)
{
	shouldRedraw = true;
}

void BaseSharedTextureMedia::renderGLInternal()
{
	if (receiver == nullptr || receiver->width == 0 || receiver->height == 0) return;

	for (int i = 0;i < extraReceivers.size() && i < extraFrameBuffers.size();i++)
	{
		OpenGLFrameBuffer* fb = extraFrameBuffers[i];
		SharedTextureReceiver* r = extraReceivers[i];
		if (fb == nullptr || r == nullptr || r->width == 0 || r->height == 0) continue;
		if (fb->getWidth() != r->width || fb->getHeight() != r->height)
		{
			shouldReinitFrameBuffer = true;
			break;
		}
	}

	if (shouldReinitFrameBuffer)
	{
		initFrameBuffer();
		shouldReinitFrameBuffer = false;
	}

	glBindTexture(GL_TEXTURE_2D, receiver->fbo->getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//draw full quad
	Init2DViewport(receiver->width, receiver->height);
	glColor3f(1, 1, 1);
	Draw2DTexRect(0, 0, receiver->width, receiver->height);

	glBindTexture(GL_TEXTURE_2D, 0);

	frameBuffer.releaseAsRenderingTarget();

	for (int i = 0;i < extraFrameBuffers.size() && i < extraReceivers.size();i++)
	{
		OpenGLFrameBuffer* fb = extraFrameBuffers[i];
		SharedTextureReceiver* r = extraReceivers[i];
		if (fb == nullptr || r == nullptr || r->width == 0 || r->height == 0) continue;
		fb->makeCurrentAndClear();

		glBindTexture(GL_TEXTURE_2D, r->fbo->getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Init2DViewport(r->width, r->height);
		glColor3f(1, 1, 1);
		Draw2DTexRect(0, 0, r->width, r->height);
		fb->releaseAsRenderingTarget();
	}

	frameBuffer.makeCurrentRenderingTarget();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void BaseSharedTextureMedia::initFrameBuffer()
{
	Media::initFrameBuffer();
	initExtraFramebuffers();
}

void BaseSharedTextureMedia::initExtraFramebuffers()
{
	for (auto& fb : extraFrameBuffers)
	{
		if (fb != nullptr)
		{
			removeFrameBuffer(getNameForFrameBuffer(fb));
			fb->release();
		}
	}
	extraFrameBuffers.clear();

	for (auto& r : extraReceivers)
	{
		OpenGLFrameBuffer* fbo = extraFrameBuffers.add(new OpenGLFrameBuffer());
		fbo->initialise(GlContextHolder::getInstance()->context, r->width, r->height);
		addFrameBuffer(r->sharingName, fbo);
	}
}

Point<int> BaseSharedTextureMedia::getDefaultMediaSize()
{
	if (receiver == nullptr) return Point<int>();
	return Point<int>(receiver->width, receiver->height);
}

void BaseSharedTextureMedia::afterLoadJSONDataInternal()
{
	setupReceiver();
}
