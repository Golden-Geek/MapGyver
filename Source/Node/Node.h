/*
  ==============================================================================

	Node.h
	Created: 15 Nov 2020 8:40:03am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class NodeConnection;
class BaseNodeViewUI;
class RootNodeManager;

#define NNLOG(t) if(logEnabled->boolValue()) NLOG(niceName, t);


class Node :
	public BaseItem,
	public NodeConnectionSlot::SlotListener
{
public:
	enum NodeType { SOURCE, FILTER, OUTPUT };

	Node(StringRef name = "Node", NodeType type = SOURCE,
		var params = var());

	virtual ~Node();

	BoolParameter* logEnabled;

	bool isInit = false;
	bool copyDataOnReceive = false;

	NodeType type;

	OwnedArray<NodeConnectionSlot, CriticalSection> inSlots;
	OwnedArray<NodeConnectionSlot, CriticalSection> outSlots;

	//Buffer data
	std::map<NodeConnectionSlot*, std::map<NodeConnection*, var>> slotNumberMap;
	std::map<NodeConnectionSlot*, std::map<NodeConnection*, String>> slotStringMap;
	std::map<NodeConnectionSlot*, std::map<NodeConnection*, Colour>> slotColorMap;
	std::map<NodeConnectionSlot*, std::map<NodeConnection*, OpenGLFrameBuffer*>> slotBufferMap;

	HashMap<NodeConnectionSlot*, NodeConnectionSlot*> passthroughMap;

	//process
	SpinLock processLock;
	bool processOnlyOnce;
	bool hasProcessed; //if it has already processed in this frame
	bool processOnlyWhenAllConnectedNodesHaveProcessed;

	//Stats
	double lastProcessTime;
	double deltaTime;
	int processTimeMS;

	//ui image safety
	SpinLock imageLock;

	//ui
	virtual String getUIInfos();
	
	virtual void clearItem() override;

	virtual void process();
	void init() { isInit = initInternal(); }

	virtual bool initInternal() { return true; }
	virtual void processInternal() {}
	virtual void processInternalPassthrough();
	virtual void processInternalPassthroughInternal() {}

	virtual void resetForNextLoop();
	virtual bool isStartingNode();

	virtual bool haveAllConnectedInputsProcessed();

	//Slots
	NodeConnectionSlot* addSlot(StringRef name, bool isInput, NodeConnectionType t);

	//IO
	virtual void receiveNumber(NodeConnectionSlot* slot, NodeConnection* nc, var value);
	virtual void receiveString(NodeConnectionSlot* slot, NodeConnection* nc, String value);
	virtual void receiveColor(NodeConnectionSlot* slot, NodeConnection* nc, Colour value);
	virtual void receiveBuffer(NodeConnectionSlot* slot, NodeConnection* nc, OpenGLFrameBuffer* buffer);


	void clearSlotMaps();

	void sendNumber(NodeConnectionSlot* slot, var value);
	void sendString(NodeConnectionSlot* slot, String value);
	void sendColor(NodeConnectionSlot* slot, Colour value);
	void sendBuffer(NodeConnectionSlot* slot, OpenGLFrameBuffer* buffer);

	bool checkConnectionCanSend(NodeConnection* c);


	//Helpers
	void addInOutSlot(NodeConnectionSlot** in, NodeConnectionSlot** out, NodeConnectionType type, StringRef inName = "In", StringRef outName = "out", bool passthrough = true);

	NodeConnectionSlot* getSlotWithName(StringRef name, bool isInput);

	var getFirstNumber(NodeConnectionSlot* slot);
	String getFirstString(NodeConnectionSlot* slot);
	Colour getFirstColor(NodeConnectionSlot* slot);
	OpenGLFrameBuffer* getFirstBuffer(NodeConnectionSlot* slot);

	Array<var> getAllNumbers();
	Array<String> getAllStrings();
	Array<Colour> getAllColors();
	Array<OpenGLFrameBuffer*> getAllBuffers();

	RootNodeManager* getRootManager();

	void checkAddNextToProcessForSlot(NodeConnectionSlot* s);
	void addNextToProcess();
	void removeNextToProcess();


	class  NodeListener
	{
	public:
		/** Destructor. */
		virtual ~NodeListener() {}
		virtual void serverControlsUpdated(Node*) {}
	};

	ListenerList<NodeListener> nodeListeners;
	void addNodeListener(NodeListener* newListener) { nodeListeners.add(newListener); }
	void removeNodeListener(NodeListener* listener) { nodeListeners.remove(listener); }

	DECLARE_ASYNC_EVENT(Node, Node, node, ENUM_LIST(INPUTS_CHANGED, OUTPUTS_CHANGED, VIEW_FILTER_UPDATED), EVENT_ITEM_CHECK);
	virtual BaseNodeViewUI* createViewUI();
	WeakReference<Node>::Master masterReference;
};