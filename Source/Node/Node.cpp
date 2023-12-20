/*
  =============================================================================

	 Node.cp
	 Created:15 Novr 200 8:40:03am
	 Author:  bkup

  =============================================================================
*/

#include "Node/NodeIncludes.h"
#include "Node.h"

Node::Node(StringRef name, NodeType type, var params) :
	BaseItem(name, true),
	type(type),
	isInit(false),
	processOnlyOnce(true),
	hasProcessed(false),
	processOnlyWhenAllConnectedNodesHaveProcessed(false),
	lastProcessTime(0),
	deltaTime(0),
	processTimeMS(0),
	nodeNotifier(5)
{

	showWarningInUI = true;

	setHasCustomColor(true);
	saveAndLoadRecursiveData = true;

	logEnabled = addBoolParameter("Log", "If enabled, this will show log messages for this node", false);

	viewUISize->setPoint(240, 80);
}

Node::~Node()
{
	slotNumberMap.clear();
	slotStringMap.clear();
	slotColorMap.clear();
	slotBufferMap.clear();

	masterReference.clear();
}

String Node::getUIInfos()
{
	return String();
}

void Node::clearItem()
{
	GenericScopedLock lock(processLock);
	BaseItem::clearItem();
	inSlots.clear();
	outSlots.clear();
	masterReference.clear();
}

void Node::process()
{
	if (hasProcessed && processOnlyOnce)
	{
		removeNextToProcess();
		return;
	}

	if (processOnlyWhenAllConnectedNodesHaveProcessed && !haveAllConnectedInputsProcessed())
	{
		removeNextToProcess();
		return;
	}


	GenericScopedLock lock(processLock);
	uint32 ms = Time::getMillisecondCounter();

	deltaTime = (ms / 1000.0) - lastProcessTime;

	try
	{
		if (!isInit) init();
		if (isInit)
		{
			if (enabled->boolValue()) processInternal();
			else processInternalPassthrough();
		}
	}
	catch (std::exception e)
	{
		NLOGERROR(niceName, "Exception during process :\n" << e.what());
	}

	uint32 t = Time::getMillisecondCounter();
	processTimeMS = t - ms;
	lastProcessTime = (t / 1000.0);

	removeNextToProcess();
	hasProcessed = true;
}

void Node::processInternalPassthrough()
{
	HashMap<NodeConnectionSlot*, NodeConnectionSlot*>::Iterator it(passthroughMap);
	while (it.next())
	{
		NodeConnectionSlot* in = it.getKey();
		NodeConnectionSlot* out = it.getValue();
		jassert(in->type == out->type);
		switch (in->type)
		{
		case NodeConnectionType::NODE_FLOAT:
		case NodeConnectionType::NODE_INT:
			sendNumber(out, getFirstNumber(in)); break;
			break;

		case NodeConnectionType::NODE_STRING:
			sendString(out, getFirstString(in)); break;

		case NodeConnectionType::NODE_COLOR:
			sendColor(out, getFirstColor(in)); break;

		case NodeConnectionType::NODE_BUFFER:
			sendBuffer(out, getFirstBuffer(in)); break;
		}
	}
	processInternalPassthroughInternal();
}

void Node::resetForNextLoop()
{
	clearSlotMaps();
	hasProcessed = false;
}

bool Node::isStartingNode()
{
	return type == SOURCE;
}

bool Node::haveAllConnectedInputsProcessed()
{
	for (auto& s : inSlots) if (!s->hasConnectedNodeProcessed(true)) return false;
	return true;
}

NodeConnectionSlot* Node::addSlot(StringRef name, bool isInput, NodeConnectionType t)
{
	jassert(getSlotWithName(name, isInput) == nullptr);
	NodeConnectionSlot* s = new NodeConnectionSlot(this, isInput, name, t);
	if (isInput) inSlots.add(s);
	else outSlots.add(s);

	s->addSlotListener(this);
	return s;
}


void Node::receiveNumber(NodeConnectionSlot* slot, NodeConnection* nc, var value)
{
	if (slotNumberMap.find(slot) == slotNumberMap.end()) slotNumberMap[slot] = std::map<NodeConnection*, var>();

	var v = copyDataOnReceive ? value.clone() : value;
	slotNumberMap[slot][nc] = v;
	checkAddNextToProcessForSlot(slot);
}

void Node::receiveString(NodeConnectionSlot* slot, NodeConnection* nc, String value)
{
	if (slotNumberMap.find(slot) == slotNumberMap.end()) slotNumberMap[slot] = std::map<NodeConnection*, var>();

	slotStringMap[slot][nc] = value;
	checkAddNextToProcessForSlot(slot);
}


void Node::receiveColor(NodeConnectionSlot* slot, NodeConnection* nc, Colour value)
{
	if (slotNumberMap.find(slot) == slotNumberMap.end()) slotNumberMap[slot] = std::map<NodeConnection*, var>();

	slotColorMap[slot][nc] = value;
	checkAddNextToProcessForSlot(slot);
}



void Node::receiveBuffer(NodeConnectionSlot* slot, NodeConnection* nc, OpenGLFrameBuffer* buffer)
{
	if (slotBufferMap.find(slot) == slotBufferMap.end()) slotBufferMap[slot] = std::map<NodeConnection*, OpenGLFrameBuffer*>();

	//var v = copyDataOnReceive ? buffer.
	slotBufferMap[slot][nc] = buffer;
	checkAddNextToProcessForSlot(slot);
}


void Node::clearSlotMaps()
{
	slotNumberMap.clear();
	slotStringMap.clear();
	slotColorMap.clear();
	slotBufferMap.clear();
}



void Node::sendNumber(NodeConnectionSlot* slot, var value)
{
	if (slot == nullptr) return;
	//if (slot->isEmpty()) return;

	for (auto& c : slot->connections)
	{
		if (!checkConnectionCanSend(c)) continue;
		//if (!c->dest->node->enabled->boolValue()) continue;
		c->dest->node->receiveNumber(c->dest, c, value);
		c->hasSentInThisLoop = true;
	}
}

void Node::sendString(NodeConnectionSlot* slot, String value)
{
	if (slot == nullptr) return;
	//if (slot->isEmpty()) return;

	for (auto& c : slot->connections)
	{
		if (!checkConnectionCanSend(c)) continue;
		//if (!c->dest->node->enabled->boolValue()) continue;
		c->dest->node->receiveString(c->dest, c, value);
		c->hasSentInThisLoop = true;
	}
}

void Node::sendColor(NodeConnectionSlot* slot, Colour value)
{
	if (slot == nullptr) return;
	//if (slot->isEmpty()) return;

	for (auto& c : slot->connections)
	{
		if (!checkConnectionCanSend(c)) continue;
		//if (!c->dest->node->enabled->boolValue()) continue;
		c->dest->node->receiveColor(c->dest, c, value);
		c->hasSentInThisLoop = true;
	}
}

void Node::sendBuffer(NodeConnectionSlot* slot, OpenGLFrameBuffer* buffer)
{
	if (slot == nullptr) return;
	//if (slot->isEmpty()) return;

	for (auto& c : slot->connections)
	{
		if (!checkConnectionCanSend(c)) continue;
		//if (!c->dest->node->enabled->boolValue()) continue;
		c->dest->node->receiveBuffer(c->dest, c, buffer);
		c->hasSentInThisLoop = true;
	}
}

bool Node::checkConnectionCanSend(NodeConnection* c)
{
	return c->enabled->boolValue() && c->dest != nullptr && c->dest->node != nullptr;
}



void Node::addInOutSlot(NodeConnectionSlot** in, NodeConnectionSlot** out, NodeConnectionType type, StringRef inName, StringRef outName, bool passthrough)
{
	*in = addSlot(inName, true, type);
	*out = addSlot(outName, false, type);
	if (passthrough) passthroughMap.set(*in, *out);
}

NodeConnectionSlot* Node::getSlotWithName(StringRef name, bool isInput)
{
	OwnedArray<NodeConnectionSlot, CriticalSection>* arr = isInput ? &inSlots : &outSlots;
	for (auto& i : *arr) if (i->name == name) return i;
	return nullptr;
}

var Node::getFirstNumber(NodeConnectionSlot* slot)
{
	if (slotNumberMap.find(slot) == slotNumberMap.end()) return 0;
	return slotNumberMap[slot].begin()->second;
}

String Node::getFirstString(NodeConnectionSlot* slot)
{
	if (slotStringMap.find(slot) == slotStringMap.end()) return String();
	return slotStringMap[slot].begin()->second;
}

Colour Node::getFirstColor(NodeConnectionSlot* slot)
{
	if (slotColorMap.find(slot) == slotColorMap.end()) return Colours::black;
	return slotColorMap[slot].begin()->second;
}

OpenGLFrameBuffer* Node::getFirstBuffer(NodeConnectionSlot* slot)
{
	if (slotBufferMap.find(slot) == slotBufferMap.end()) return nullptr;
	return slotBufferMap[slot].begin()->second;
}


Array<var> Node::getAllNumbers()
{
	Array<var> result;
	for (auto ca : slotNumberMap) for (auto c : ca.second) result.add(c.second);
	return result;
}

Array<String> Node::getAllStrings()
{
	Array<String> result;
	for (auto ca : slotStringMap) for (auto c : ca.second) result.add(c.second);
	return result;
}

Array<Colour> Node::getAllColors()
{
	Array<Colour> result;
	for (auto ca : slotColorMap) for (auto c : ca.second) result.add(c.second);
	return result;
}

Array<OpenGLFrameBuffer*> Node::getAllBuffers()
{
	Array<OpenGLFrameBuffer*> result;
	for (auto ca : slotBufferMap) for (auto c : ca.second) result.add(c.second);
	return result;
}

RootNodeManager* Node::getRootManager()
{
	return ControllableUtil::findParentAs<RootNodeManager>(this);
}

void Node::checkAddNextToProcessForSlot(NodeConnectionSlot* slot)
{
	if (!slot->processOnReceive) return;
	if (hasProcessed && processOnlyOnce) return;

	addNextToProcess();
}

void Node::addNextToProcess()
{
	if (Engine::mainEngine->isClearing) return;
	getRootManager()->nextToProcess.addIfNotAlreadyThere(this);
}

void Node::removeNextToProcess()
{
	if (Engine::mainEngine->isClearing) return;
	getRootManager()->nextToProcess.removeAllInstancesOf(this);
}


BaseNodeViewUI* Node::createViewUI()
{
	return new BaseNodeViewUI(this);
}