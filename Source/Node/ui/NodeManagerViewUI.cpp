/*
  ==============================================================================

	NodeManagerViewUI.cpp
	Created: 15 Nov 2020 8:40:22am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"
#include "Media/MediaIncludes.h"

NodeManagerViewUI::NodeManagerViewUI(NodeManager* manager) :
	BaseManagerViewUI(manager->niceName, manager)
{
	addExistingItems(false);

	connectionManagerUI.reset(new NodeConnectionManagerViewUI(this, manager->connectionManager.get()));
	addAndMakeVisible(connectionManagerUI.get(), 0);

	setShowPane(true);

	bringToFrontOnSelect = false;
	enableSnapping = false;

	updatePositionOnDragMove = true;

	removeMouseListener(this);
	addMouseListener(this, true);

	ViewStatsTimer::getInstance()->addListener(this);
	statsLabel.setFont(FontOptions(12));
	statsLabel.setColour(statsLabel.textColourId, NORMAL_COLOR.brighter(.6f));
	statsLabel.setOpaque(false);
	statsLabel.setJustificationType(Justification::centredRight);
	addAndMakeVisible(statsLabel);


}

NodeManagerViewUI::~NodeManagerViewUI()
{
	if (ViewStatsTimer* vs = ViewStatsTimer::getInstanceWithoutCreating()) vs->removeListener(this);
}

BaseNodeViewUI* NodeManagerViewUI::createUIForItem(Node* n)
{
	return n->createViewUI();
}

void NodeManagerViewUI::resized()
{
	BaseManagerViewUI::resized();
	connectionManagerUI->setBounds(getLocalBounds());
	connectionManagerUI->resized();
}

void NodeManagerViewUI::resizedInternalHeader(Rectangle<int>& r)
{
	BaseManagerViewUI::resizedInternalHeader(r);
	statsLabel.setBounds(r.removeFromRight(300));
}

void NodeManagerViewUI::mouseDown(const MouseEvent& e)
{
	if (NodeConnector* c = dynamic_cast<NodeConnector*>(e.originalComponent))  connectionManagerUI->startCreateConnection(c);
	else if (NodeConnectionViewUI::Handle* h = dynamic_cast<NodeConnectionViewUI::Handle*>(e.originalComponent))
	{
		NodeConnectionViewUI* cui = dynamic_cast<NodeConnectionViewUI*>(h->getParentComponent());
		NodeConnector* cc = h == &cui->sourceHandle ? cui->destConnector : cui->sourceConnector;
		connectionManagerUI->startCreateConnection(cc, cui->item);
	}
	else if (e.eventComponent == this) BaseManagerViewUI::mouseDown(e);
}

void NodeManagerViewUI::mouseDrag(const MouseEvent& e)
{
	if (connectionManagerUI->tmpConnectionUI != nullptr) connectionManagerUI->updateCreateConnection();
	else if (e.eventComponent == this) BaseManagerViewUI::mouseDrag(e);
}

void NodeManagerViewUI::mouseUp(const MouseEvent& e)
{
	if (connectionManagerUI->tmpConnectionUI != nullptr) connectionManagerUI->endCreateConnection();
	else if (e.eventComponent == this) BaseManagerViewUI::mouseUp(e);
}

void NodeManagerViewUI::refreshStats()
{
	if (inspectable.wasObjectDeleted()) return;
	if (RootNodeManager* n = dynamic_cast<RootNodeManager*>(manager))
	{
		statsLabel.setText("Process : " + String(n->processTimeMS) + "ms - Framerate : " + String(n->averageFPS) + " fps (Max : " + String(n->maxFPS) + ")", dontSendNotification);
	}
}

NodeConnector* NodeManagerViewUI::getCandidateConnector(bool lookForInput, NodeConnectionType connectionType, BaseNodeViewUI* excludeUI)
{
	for (auto& ui : itemsUI)
	{
		if (ui == excludeUI) continue;
		if (!ui->isVisible()) continue;

		Array<NodeConnector*> connectors = ui->getConnectorsForType(connectionType, lookForInput);

		for (auto& c : connectors)
		{
			if (c->getLocalBounds().expanded(10).contains(c->getMouseXYRelative())) return c;
		}
	}

	return nullptr;
}

NodeManagerViewPanel::NodeManagerViewPanel(StringRef contentName) :
	ShapeShifterContentComponent(contentName)
{
	contentIsFlexible = true;
	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);

}

NodeManagerViewPanel::~NodeManagerViewPanel()
{
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);
}

void NodeManagerViewPanel::setManager(NodeManager* manager)
{
	if (managerUI != nullptr)
	{
		if (managerUI->manager == manager) return;
		removeChildComponent(managerUI.get());
	}

	if (manager != nullptr) managerUI.reset(new NodeManagerViewUI(manager));
	else managerUI.reset();

	if (managerUI != nullptr)
	{
		addAndMakeVisible(managerUI.get());
	}

	updateCrumbs();
	resized();

	if (NodeManagerPanel* p = ShapeShifterManager::getInstance()->getContentForType<NodeManagerPanel>())
	{
		p->setManager(manager);
	}

}

void NodeManagerViewPanel::updateCrumbs()
{
	for (auto& c : crumbsBT) removeChildComponent(c);
	crumbsBT.clear();
	crumbManagers.clear();

	if (managerUI == nullptr || managerUI->manager == nullptr) return;

	ControllableContainer* pc = managerUI->manager;
	while (pc != nullptr)
	{
		if (NodeManager* m = dynamic_cast<NodeManager*>(pc))
		{
			RootNodeManager* sm = dynamic_cast<RootNodeManager*>(m);
			String n = sm != nullptr ? sm->media->niceName : m->parentContainer->niceName; //container's name

			TextButton* bt = new TextButton(n);
			bt->addListener(this);
			bt->setEnabled(m != managerUI->manager);
			addAndMakeVisible(bt);

			crumbsBT.insert(0, bt);
			crumbManagers.insert(0, m);
		}

		pc = pc->parentContainer;
	}
}

void NodeManagerViewPanel::resized()
{
	if (managerUI != nullptr) managerUI->setBounds(getLocalBounds());
	Rectangle<int> hr = getLocalBounds().removeFromTop(24);
	hr.removeFromRight(30);
	for (auto& c : crumbsBT) c->setBounds(hr.removeFromLeft(80).reduced(2));
}

void NodeManagerViewPanel::buttonClicked(Button* b)
{
	if (crumbsBT.contains((TextButton*)b))
	{
		setManager(crumbManagers[crumbsBT.indexOf((TextButton*)b)]);
	}
}

void NodeManagerViewPanel::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == e.SELECTION_CHANGED)
	{
		if (NodeMedia* nm = InspectableSelectionManager::mainSelectionManager->getInspectableAs<NodeMedia>())
		{
			setManager(nm->nodes.get());
		}
	}
}
