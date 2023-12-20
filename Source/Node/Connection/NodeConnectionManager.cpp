/*
  ==============================================================================

    NodeConnectionManager.cpp
    Created: 16 Nov 2020 10:00:12am
    Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

NodeConnectionManager::NodeConnectionManager(NodeManager * nodeManager) :
    BaseManager("Connections"),
    nodeManager(nodeManager)
{
}

NodeConnectionManager::~NodeConnectionManager()
{
}


NodeConnection* NodeConnectionManager::createItem()
{
    return new NodeConnection(nodeManager);
}

void NodeConnectionManager::addConnection(NodeConnectionSlot* source, NodeConnectionSlot* dest)
{
    UndoMaster::getInstance()->performActions("Add Connection", getAddConnectionUndoableAction(source, dest));
}

Array<UndoableAction*> NodeConnectionManager::getAddConnectionUndoableAction(NodeConnectionSlot* source, NodeConnectionSlot* dest)
{
    if (getConnectionForSourceAndDest(source, dest))
    {
        LOGWARNING("Connection already exists !");
        return Array<UndoableAction*>();
    }

    NodeConnection* connection = new NodeConnection(nodeManager, source, dest);
    

    if (connection == nullptr) return Array<UndoableAction*>();

    return getAddItemUndoableAction(connection);
}

NodeConnection* NodeConnectionManager::getConnectionForSourceAndDest(NodeConnectionSlot* source, NodeConnectionSlot* dest)
{
    for (auto& c : items) if (c->source == source && c->dest == dest) return c;
    return nullptr;
}

Array<UndoableAction*> NodeConnectionManager::getRemoveAllLinkedConnectionsActions(Array<Node*> itemsToRemove)
{
    Array<NodeConnection*> connectionsToRemove;

    for (auto& c : items)
    {
        if (itemsToRemove.contains(c->source->node) || itemsToRemove.contains(c->dest->node)) connectionsToRemove.addIfNotAlreadyThere(c);
    }
    
    Array<UndoableAction*> actions;
    actions.addArray(getRemoveItemsUndoableAction(connectionsToRemove));
    return actions;
}

void NodeConnectionManager::afterLoadJSONDataInternal()
{
    BaseManager::afterLoadJSONDataInternal();

    Array<NodeConnection*> toRemove;
    for (auto& i : items) if (i->source == nullptr || i->dest == nullptr) toRemove.add(i);
    for (auto& i : toRemove) removeItem(i);
}
