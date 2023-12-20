/*
  ==============================================================================

	NodeConnectionManagerViewUI.cpp
	Created: 16 Nov 2020 10:00:57am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

NodeConnectionManagerViewUI::NodeConnectionManagerViewUI(NodeManagerViewUI* nodeManagerUI, NodeConnectionManager* manager) :
	BaseManagerUI(manager->niceName, manager, false),
	nodeManagerUI(nodeManagerUI)
{
	bringToFrontOnSelect = false; 
	autoFilterHitTestOnItems = true;
	validateHitTestOnNoItem = false; 

	setRepaintsOnMouseActivity(true);

	transparentBG = true;
	animateItemOnAdd = false;
	setInterceptsMouseClicks(false, true);
	removeChildComponent(addItemBT.get());

	addExistingItems();
}

NodeConnectionManagerViewUI::~NodeConnectionManagerViewUI()
{
}

 NodeConnectionViewUI* NodeConnectionManagerViewUI::createUIForItem(NodeConnection* item)
{
	 jassert(!item->source->node.wasObjectDeleted() && !item->dest->node.wasObjectDeleted());

	BaseNodeViewUI* sourceUI = nodeManagerUI->getUIForItem(item->source->node);
	BaseNodeViewUI* destUI = nodeManagerUI->getUIForItem(item->dest->node);
	NodeConnector* sourceConnector = sourceUI->getConnectorForSlot(item->source);
	NodeConnector* destConnector = destUI->getConnectorForSlot(item->dest);

	return new NodeConnectionViewUI(item, sourceConnector, destConnector);
}

void NodeConnectionManagerViewUI::placeItems(Rectangle<int> &r)
{
	for (auto& ui : itemsUI) ui->updateBounds();
}

void NodeConnectionManagerViewUI::addItemUIInternal(NodeConnectionViewUI* ui)
{
	ui->updateBounds();
}

void NodeConnectionManagerViewUI::resized()
{
}

void NodeConnectionManagerViewUI::startCreateConnection(NodeConnector* connector, NodeConnection* connectionToReplace)
{
	if (connector == nullptr) return;

	tmpConnectionToReplace = connectionToReplace;
	if (NodeConnectionViewUI * cui = getUIForItem(tmpConnectionToReplace)) cui->setAlpha(.5f);
	tmpConnectionLookForInput = !connector->isInput;
	tmpConnectionUI.reset(new NodeConnectionViewUI(nullptr, connector->isInput  ? nullptr : connector, connector->isInput ? connector : nullptr));
	addAndMakeVisible(tmpConnectionUI.get());
}

void NodeConnectionManagerViewUI::updateCreateConnection()
{
	if (tmpConnectionUI == nullptr) return;

	NodeConnector* baseConnector = tmpConnectionLookForInput ? tmpConnectionUI->sourceConnector : tmpConnectionUI->destConnector;
	NodeConnector* candidateConnector = nodeManagerUI->getCandidateConnector(tmpConnectionLookForInput, baseConnector->slot->type, baseConnector->nodeViewUI);


	if (tmpConnectionLookForInput) tmpConnectionUI->setDestConnector(candidateConnector);
	else tmpConnectionUI->setSourceConnector(candidateConnector);
	tmpConnectionUI->updateBounds();
}


void NodeConnectionManagerViewUI::endCreateConnection()
{
	if (tmpConnectionUI == nullptr) return;
	
	if (tmpConnectionUI->sourceConnector != nullptr && tmpConnectionUI->destConnector != nullptr) 
	{
		if (tmpConnectionToReplace != nullptr
			&& tmpConnectionUI->sourceConnector->nodeViewUI->item == tmpConnectionToReplace->source->node
			&& tmpConnectionUI->destConnector->nodeViewUI->item == tmpConnectionToReplace->dest->node)
		{
			//same, do nothing
		}
		else
		{
			Array<UndoableAction*> actions;
			actions.addArray(manager->getAddConnectionUndoableAction(tmpConnectionUI->sourceConnector->slot, tmpConnectionUI->destConnector->slot));
			UndoMaster::getInstance()->performActions(tmpConnectionToReplace != nullptr?"Move Connection":"Add Connection", actions);
		}
	}

	if (NodeConnectionViewUI* cui = getUIForItem(tmpConnectionToReplace)) cui->setAlpha(1);

	tmpConnectionToReplace = nullptr;
	removeChildComponent(tmpConnectionUI.get());
	tmpConnectionUI.reset();
}