/*
  ==============================================================================

	MediaNode.cpp
	Created: 2 Dec 2022 10:23:46am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"
#include "Media/MediaIncludes.h"

MediaNode::MediaNode(var params) :
	Node(getTypeString(), Node::SOURCE)
{
	source = addTargetParameter("Media", "The media to use for this", MediaManager::getInstance());
	source->targetType = source->CONTAINER;
	source->maxDefaultSearchLevel = 0;

	outBuffer = addSlot("Out Buffer", false, NODE_BUFFER);
}

MediaNode::~MediaNode()
{
}

bool MediaNode::initInternal()
{
	return true;
}

void MediaNode::processInternal()
{
	if (source->targetContainer == nullptr || source->targetContainer.wasObjectDeleted()) return;

	Media* s = dynamic_cast<Media*>(source->targetContainer.get());

	if (s == nullptr) return;
}