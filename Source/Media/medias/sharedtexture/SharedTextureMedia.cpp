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
	const ScopedLock lock(receiverLock);
	auto* manager = SharedTextureManager::getInstanceWithoutCreating();
	if (manager == nullptr) return;
	if (receiver != nullptr)
	{
		receiver->removeListener(this);
		manager->removeReceiver(receiver);
	}

	for (auto& r : extraReceivers)
	{
		if (r != nullptr)
		{
			r->removeListener(this);
			manager->removeReceiver(r);
		}
	}
}


void BaseSharedTextureMedia::setupReceiver()
{
	const ScopedLock lock(receiverLock);
	if (receiver != nullptr)
	{
		receiver->removeListener(this);
		SharedTextureManager::getInstance()->removeReceiver(receiver);
	}
	// An empty Spout name selects the active sender, which may be another app.
	receiver = sharingName->stringValue().isEmpty() ? nullptr : SharedTextureManager::getInstance()->addReceiver(sharingName->stringValue());
	if (receiver != nullptr) receiver->addListener(this);
	shouldRedraw = true;
}

void BaseSharedTextureMedia::setupExtraReceivers(Array<SharedTextureReceiver*> receivers)
{
	const ScopedLock lock(receiverLock);
	if (extraReceivers == receivers) return;
	for (auto& r : extraReceivers)
	{
		if (r != nullptr && !receivers.contains(r))
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
	shouldRedraw = true;
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
	const ScopedLock lock(receiverLock);
	if (receiver == nullptr || receiver->fbo == nullptr || receiver->width == 0 || receiver->height == 0) return;

	for (int i = 0;i < extraReceivers.size() && i < extraFrameBuffers.size();i++)
	{
		OpenGLFrameBuffer* fb = extraFrameBuffers[i];
		SharedTextureReceiver* r = extraReceivers[i];
		if (fb == nullptr || r == nullptr || r->width == 0 || r->height == 0) continue;
		if (!fb->isValid() || fb->getWidth() != r->width || fb->getHeight() != r->height)
		{
			shouldReinitFrameBuffer = true;
			break;
		}
	}

	if (shouldReinitFrameBuffer)
	{
		// The main FBO is already bound by Media::renderOpenGLMedia. Rebuilding
		// only the extra FBOs keeps that rendering target alive during refresh.
		initExtraFramebuffers();
	}

	glBindTexture(GL_TEXTURE_2D, receiver->fbo->getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//draw full quad
	Init2DViewport(receiver->width, receiver->height);
	glColor3f(1, 1, 1);
#if JUCE_WINDOWS
	// Spout textures need the opposite vertical orientation in our media FBO.
	Draw2DTexRectFlipped(0, 0, receiver->width, receiver->height);
#else
	Draw2DTexRect(0, 0, receiver->width, receiver->height);
#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	frameBuffer.releaseAsRenderingTarget();

	for (int i = 0;i < extraFrameBuffers.size() && i < extraReceivers.size();i++)
	{
		OpenGLFrameBuffer* fb = extraFrameBuffers[i];
		SharedTextureReceiver* r = extraReceivers[i];
		if (fb == nullptr || !fb->isValid() || r == nullptr || r->fbo == nullptr || r->width == 0 || r->height == 0) continue;
		fb->makeCurrentAndClear();

		glBindTexture(GL_TEXTURE_2D, r->fbo->getTextureID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Init2DViewport(r->width, r->height);
		glColor3f(1, 1, 1);
#if JUCE_WINDOWS
		Draw2DTexRectFlipped(0, 0, r->width, r->height);
#else
		Draw2DTexRect(0, 0, r->width, r->height);
#endif
		fb->releaseAsRenderingTarget();
	}

	frameBuffer.makeCurrentRenderingTarget();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void BaseSharedTextureMedia::initFrameBuffer()
{
	const ScopedLock lock(receiverLock);
	Media::initFrameBuffer();
	initExtraFramebuffers();
}

void BaseSharedTextureMedia::initExtraFramebuffers()
{
	const ScopedLock lock(receiverLock);
	for (auto& fb : extraFrameBuffers)
	{
		if (fb != nullptr)
		{
			removeFrameBuffer(getNameForFrameBuffer(fb), false);
			fb->release();
		}
	}
	extraFrameBuffers.clear();

	for (auto& r : extraReceivers)
	{
		OpenGLFrameBuffer* fbo = extraFrameBuffers.add(new OpenGLFrameBuffer());
		if (r->width > 0 && r->height > 0)
			fbo->initialise(GlContextHolder::getInstance()->context, r->width, r->height);
		addFrameBuffer(r->sharingName, fbo, false);
	}

	mediaNotifier.addMessage(new MediaEvent(MediaEvent::SUB_FRAMEBUFFERS_CHANGED, this));
	shouldReinitFrameBuffer = false;
}

Point<int> BaseSharedTextureMedia::getDefaultMediaSize()
{
	const ScopedLock lock(receiverLock);
	if (receiver == nullptr) return Point<int>();
	return Point<int>(receiver->width, receiver->height);
}

void BaseSharedTextureMedia::afterLoadJSONDataInternal()
{
	setupReceiver();
}
