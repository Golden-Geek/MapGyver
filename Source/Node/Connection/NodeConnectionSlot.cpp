/*
  ==============================================================================

	NodeConnectionSlot.cpp
	Created: 5 Apr 2022 11:37:07am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

NodeConnectionSlot::NodeConnectionSlot(Node* node, bool isInput, String name, NodeConnectionType type) :
	isInput(isInput),
	processOnReceive(true),
	name(name),
	type(type),
	node(node)
{

}

NodeConnectionSlot::~NodeConnectionSlot()
{
	while (connections.size() > 0) removeConnection(connections[0]);
}

void NodeConnectionSlot::addConnection(NodeConnection* c)
{
	if (c == nullptr) return;
	if (connections.contains(c)) return;

	connections.add(c);
	if (isInput) c->setDest(this);
	else c->setSource(this);

	slotListeners.call(&SlotListener::connectionAdded, this, c);
}

void NodeConnectionSlot::removeConnection(NodeConnection* c)
{
	if (c == nullptr) return;
	if (!connections.contains(c)) return;

	connections.removeAllInstancesOf(c);
	if (isInput) c->setDest(nullptr);
	else c->setSource(nullptr);

	slotListeners.call(&SlotListener::connectionRemoved, this, c);
}

bool NodeConnectionSlot::isConnectedTo(NodeConnectionSlot* s)
{
	for (auto& c : connections)
	{
		NodeConnectionSlot* ss = isInput ? c->dest : c->source;
		if (ss == s) return true;
	}

	return false;
}

bool NodeConnectionSlot::hasConnectedNodeProcessed(bool trueIfEmpty)
{
	if (isEmpty()) return trueIfEmpty;
	if (!connections[0]->enabled->boolValue() || connections[0]->source == nullptr || connections[0]->source->node == nullptr) return trueIfEmpty;
	return connections[0]->source->node->hasProcessed;
}
