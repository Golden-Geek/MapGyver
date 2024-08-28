/*
  ==============================================================================

	NodeConnection.h
	Created: 16 Nov 2020 10:00:05am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class Node;
class NodeManager;

class NodeConnection :
	public BaseItem,
	public Inspectable::InspectableListener//,
	//public Node::NodeListener
{
public:
	
	NodeConnection(NodeManager* nodeManager = nullptr, NodeConnectionSlot* source = nullptr, NodeConnectionSlot* dest = nullptr);
	virtual ~NodeConnection();

	NodeManager* nodeManager;

	NodeConnectionType connectionType;
	NodeConnectionSlot* source;
	NodeConnectionSlot* dest;

	bool hasSentInPrevLoop;
	bool hasSentInThisLoop;

	virtual void setSource(NodeConnectionSlot* node);
	virtual void setDest(NodeConnectionSlot* node);

	virtual void handleNodesUpdated();

	void inspectableDestroyed(Inspectable*) override;

	var getJSONData() override;
	void loadJSONDataItemInternal(var data) override;
	virtual void loadJSONDataConnectionInternal(var data) {}

	static Colour getColorForType(NodeConnectionType t);

	DECLARE_ASYNC_EVENT(NodeConnection, Connection, connection, ENUM_LIST(SOURCE_CHANGED, DEST_CHANGED), EVENT_ITEM_CHECK);
};