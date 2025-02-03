/*
  =============================================================================

	 NodeManager.cp
	 Created:15 Novr 200 8:39:59am
	 Author:  bkup

  =============================================================================
*/

#include "Node/NodeIncludes.h"


NodeManager::NodeManager(NodeMedia* media) :
	BaseManager("Nodes"),
	media(media)
{
	managerFactory = NodeFactory::getInstance();

	connectionManager.reset(new NodeConnectionManager(this));
	connectionManager->hideInRemoteControl = true;
	connectionManager->defaultHideInRemoteControl = true;
	addChildControllableContainer(connectionManager.get());
}


NodeManager::~NodeManager()
{
}


void NodeManager::clear()
{
	connectionManager->clear();
	BaseManager::clear();
}


void NodeManager::addItemInternal(Node* item, var data)
{
	item->addNodeListener(this);
}

void NodeManager::removeItemInternal(Node* item)
{
	item->removeNodeListener(this);
}


Array<UndoableAction*> NodeManager::getRemoveItemUndoableAction(Node* item)
{
	Array<UndoableAction*> result;
	Array<Node*> itemsToRemove;
	itemsToRemove.add(item);
	result.addArray(connectionManager->getRemoveAllLinkedConnectionsActions(itemsToRemove));
	result.addArray(BaseManager::getRemoveItemUndoableAction(item));
	return result;
}

Array<UndoableAction*> NodeManager::getRemoveItemsUndoableAction(Array<Node*> itemsToRemove)
{
	Array<UndoableAction*> result;
	result.addArray(connectionManager->getRemoveAllLinkedConnectionsActions(itemsToRemove));
	result.addArray(BaseManager::getRemoveItemsUndoableAction(itemsToRemove));
	return result;
}


var NodeManager::getJSONData(bool includeNonOverriden)
{
	var data = BaseManager::getJSONData(includeNonOverriden);
	data.getDynamicObject()->setProperty(connectionManager->shortName, connectionManager->getJSONData());
	return data;
}

void NodeManager::loadJSONDataManagerInternal(var data)
{
	BaseManager::loadJSONDataManagerInternal(data);
	connectionManager->loadJSONData(data.getProperty(connectionManager->shortName, var()));
}


//ROOT
RootNodeManager::RootNodeManager(NodeMedia* media) :
	NodeManager(media),
	Thread("Nodes"),
	processTimeMS(1),
	averageFPS(0)
{
	Engine::mainEngine->addEngineListener(this);
	fps = addIntParameter("FPS", "Target process rate", 30, 1, 500);
}

RootNodeManager::~RootNodeManager()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
	stopThread(1000);
}


void RootNodeManager::addItemInternal(Node* item, var data)
{
	GenericScopedLock lock(itemLoopLock);
	NodeManager::addItemInternal(item, data);
}

void RootNodeManager::removeItemInternal(Node* item)
{
	GenericScopedLock lock(itemLoopLock);
	NodeManager::removeItemInternal(item);
}


void RootNodeManager::clear()
{
	stopThread(1000);
	NodeManager::clear();
}

void RootNodeManager::run()
{
	wait(1000); //safety

	long lastFrameTime = Time::getMillisecondCounter();

	nextToProcess.clear();

	while (!threadShouldExit())
	{
		long millis = Time::getMillisecondCounter();

		try
		{
			{
				GenericScopedLock lock(itemLoopLock);
				for (auto& i : items) i->resetForNextLoop();
				for (auto& i : connectionManager->items)
				{
					i->hasSentInPrevLoop = i->hasSentInThisLoop;
					i->hasSentInThisLoop = false;
				}

				for (auto& i : items)
				{
					if (i->isStartingNode()) i->process();
				}
				while (!threadShouldExit() && !nextToProcess.isEmpty())
				{
					Array<Node*> processList;
					processList.addArray(nextToProcess);
					for (auto& n : processList)
					{
						if (threadShouldExit()) return;
						n->process();
					}
				}
			}
		}
		catch (std::exception e)
		{
			LOGERROR("Error during process : " << e.what());
			nextToProcess.clear();

			return;
		}

		uint32 t = Time::getMillisecondCounter();
		processTimeMS = t - millis;
		maxFPS = 1000 / jmax(processTimeMS, 1);

		int frameDiff = t - lastFrameTime;
		averageFPS = 1000 / jmax(frameDiff, 1);
		lastFrameTime = t;

		int targetFrameMS = 1000 / fps->intValue();
		int timeToWait = targetFrameMS - processTimeMS;
		if (timeToWait > 0) wait(timeToWait);
	}

	nextToProcess.clear();

}

void RootNodeManager::startLoadFile()
{
	startThread();
}


void RootNodeManager::afterLoadJSONDataInternal()
{
	NodeManager::afterLoadJSONDataInternal();
	startThread();
}