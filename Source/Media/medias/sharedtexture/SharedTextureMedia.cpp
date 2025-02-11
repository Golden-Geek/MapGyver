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
	receiver(nullptr)
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
}


void BaseSharedTextureMedia::setupReceiver()
{
	if(receiver != nullptr) SharedTextureManager::getInstance()->removeReceiver(receiver);
	receiver = SharedTextureManager::getInstance()->addReceiver(sharingName->stringValue());
	if(receiver != nullptr) receiver->addListener(this);
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


	glBindTexture(GL_TEXTURE_2D, receiver->fbo->getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//draw full quad
	Init2DViewport(receiver->width, receiver->height);
	glColor3f(1, 1, 1);
	Draw2DTexRect(0, 0, receiver->width, receiver->height);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Point<int> BaseSharedTextureMedia::getMediaSize()
{
	if (receiver == nullptr) return Point<int>();
	return Point<int>(receiver->width, receiver->height);
}

void BaseSharedTextureMedia::afterLoadJSONDataInternal()
{
	setupReceiver();
}
