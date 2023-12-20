/*
  ==============================================================================

	NodeManagerUI.cpp
	Created: 15 Nov 2020 8:40:16am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"
#include "Media/MediaIncludes.h"

NodeManagerUI::NodeManagerUI(NodeManager* manager) :
	BaseManagerUI(manager->niceName, manager)
{
	addExistingItems();
}

NodeManagerUI::~NodeManagerUI()
{
}

NodeManagerPanel::NodeManagerPanel(StringRef contentName) :
	ShapeShifterContentComponent(contentName)
{
	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);
}

NodeManagerPanel::~NodeManagerPanel()
{
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);
}

void NodeManagerPanel::setManager(NodeManager* manager)
{
	if (managerUI != nullptr)
	{
		if (managerUI->manager == manager) return;
		removeChildComponent(managerUI.get());
	}

	if (manager != nullptr) managerUI.reset(new NodeManagerUI(manager));
	else managerUI.reset();

	if (managerUI != nullptr)
	{
		addAndMakeVisible(managerUI.get());
	}

	resized();
}

void NodeManagerPanel::resized()
{
	if (managerUI != nullptr) managerUI->setBounds(getLocalBounds());
}

void NodeManagerPanel::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == e.SELECTION_CHANGED)
	{
		if (NodeMedia* nm = InspectableSelectionManager::mainSelectionManager->getInspectableAs<NodeMedia>())
		{
			setManager(nm->nodes.get());
		}
	}
}
