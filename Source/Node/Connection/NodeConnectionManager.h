/*
  ==============================================================================

    NodeConnectionManager.h
    Created: 16 Nov 2020 10:00:12am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class NodeManager;

class NodeConnectionManager :
    public BaseManager<NodeConnection>
{
public:
    NodeConnectionManager(NodeManager * nodeManager);
    ~NodeConnectionManager();

    NodeManager* nodeManager;

    NodeConnection* createItem() override;

    void addConnection(NodeConnectionSlot* source, NodeConnectionSlot* dest);
    Array<UndoableAction*> getAddConnectionUndoableAction(NodeConnectionSlot* source, NodeConnectionSlot* dest);
    virtual NodeConnection* getConnectionForSourceAndDest(NodeConnectionSlot* source, NodeConnectionSlot* dest);
    Array<UndoableAction*> getRemoveAllLinkedConnectionsActions(Array<Node*> itemsToRemove);

    void afterLoadJSONDataInternal() override;
};