/*
  ==============================================================================

	CropboxNode.cpp
	Created: 5 Apr 2022 10:45:43am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

ShapeNode::ShapeNode(var params) :
	Node(getTypeString(), FILTER, params)
{

	addSlot("Out", false, NODE_BUFFER);
}

ShapeNode::~ShapeNode()
{
}

void ShapeNode::processInternal()
{
}
