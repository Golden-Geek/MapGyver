/*
  ==============================================================================

    NodeManagerUI.h
    Created: 15 Nov 2020 8:40:16am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class NodeManagerUI :
    public BaseManagerUI<NodeManager, Node, NodeUI>
{
public:
    NodeManagerUI(NodeManager * manager);
    ~NodeManagerUI();

};

class NodeManagerPanel :
    public ShapeShifterContentComponent,
    public InspectableSelectionManager::AsyncListener
{
public:
    NodeManagerPanel(StringRef contentName);
    ~NodeManagerPanel();

    std::unique_ptr<NodeManagerUI> managerUI;

    void setManager(NodeManager* manager);
    void resized() override;

    void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

    static NodeManagerPanel* create(const String& name) { return new NodeManagerPanel(name); }

};