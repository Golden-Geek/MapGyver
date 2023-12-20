/*
  ==============================================================================

    NodeMedia.cpp
    Created: 28 Nov 2023 12:07:51pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Node/NodeIncludes.h"

NodeMedia::NodeMedia(var params) :
	Media(getTypeString(), params)
{
	nodes.reset(new RootNodeManager(this));
	addChildControllableContainer(nodes.get());
}

NodeMedia::~NodeMedia()
{
}


void NodeMedia::renderGLInternal()
{
	//glBindTexture(GL_TEXTURE_2D, receiver->fbo->getTextureID());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	////draw full quad
	//Init2DViewport(receiver->width, receiver->height);
	//glColor3f(1, 1, 1);
	//Draw2DTexRect(0, 0, receiver->width, receiver->height);

	//glBindTexture(GL_TEXTURE_2D, 0);
}

Point<int> NodeMedia::getMediaSize()
{
	return Point<int>();
}