/*
  ==============================================================================

	NodeConnectionSlot.h
	Created: 5 Apr 2022 11:37:07am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class Node;
class NodeConnection;

enum NodeConnectionType {UNKNOWN, NODE_BOOL, NODE_FLOAT, NODE_INT, NODE_STRING, NODE_COLOR, NODE_BUFFER };

class NodeConnectionSlot
{
public:
	NodeConnectionSlot(Node* node, bool isInput, String name, NodeConnectionType type);
	~NodeConnectionSlot();

	bool isInput;
	bool processOnReceive;

	String name;
	NodeConnectionType type;

	WeakReference<Node> node;
	Array<NodeConnection*> connections;

	void addConnection(NodeConnection* c);
	void removeConnection(NodeConnection * c);

	bool isConnectedTo(NodeConnectionSlot* s);

	bool hasConnectedNodeProcessed(bool trueIfEmpty = true);

	bool isEmpty() const { return connections.size() == 0; }

	class  SlotListener
	{
	public:
		/** Destructor. */
		virtual ~SlotListener() {}
		virtual void connectionAdded(NodeConnectionSlot*, NodeConnection*) {}
		virtual void connectionRemoved(NodeConnectionSlot*, NodeConnection*) {}
	};

	ListenerList<SlotListener> slotListeners;
	void addSlotListener(SlotListener* newListener) { slotListeners.add(newListener); }
	void removeSlotListener(SlotListener* listener) { slotListeners.remove(listener); }
};
