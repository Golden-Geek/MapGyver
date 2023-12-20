/*
  ==============================================================================

	NodeConnector.cpp
	Created: 16 Nov 2020 3:03:26pm
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

NodeConnector::NodeConnector(NodeConnectionSlot* slot, BaseNodeViewUI * nodeViewUI) :
	nodeViewUI(nodeViewUI),
	slot(slot),
	isInput(slot->isInput)
{
	setRepaintsOnMouseActivity(true);
	if(!slot->node.wasObjectDeleted()) setTooltip(slot->node->niceName + "." + slot->name);
}

NodeConnector::~NodeConnector()
{
}

void NodeConnector::paint(Graphics& g)
{
	Colour c = NodeConnection::getColorForType(slot->type);
	g.setColour(c.brighter(isMouseOverOrDragging() ? .3f : 0));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 2);
}