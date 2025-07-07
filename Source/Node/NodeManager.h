/*
  ==============================================================================

	NodeManager.h
	Created: 15 Nov 2020 8:39:59am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class NodeConnectionManager;
class NodeMedia;

class NodeManager :
	public Manager<Node>,
	public Node::NodeListener
{
public:
	NodeManager(NodeMedia* media);
	~NodeManager();

	NodeMedia* media;

	std::unique_ptr<NodeConnectionManager> connectionManager;

	void clear() override;

	virtual Array<UndoableAction*> getRemoveItemUndoableAction(Node* n) override;
	virtual Array<UndoableAction*> getRemoveItemsUndoableAction(Array<Node*> n) override;

	virtual void addItemInternal(Node* item, var data) override;
	virtual void removeItemInternal(Node* item) override;

	var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataManagerInternal(var data) override;
};


class RootNodeManager :
	public NodeManager,
	public EngineListener,
	public Thread
{
public:
	RootNodeManager(NodeMedia* media);
	~RootNodeManager();

	Array<Node*, CriticalSection> nextToProcess;

	IntParameter* fps;
	int processTimeMS;
	int averageFPS;
	int maxFPS;

	SpinLock itemLoopLock;

	void clear() override;

	void run() override;

	void addItemInternal(Node* item, var data) override;
	void removeItemInternal(Node* item) override;

	void startLoadFile() override;


	void afterLoadJSONDataInternal() override;
};