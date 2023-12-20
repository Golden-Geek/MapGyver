/*
  ==============================================================================

    NodeManagerViewUI.h
    Created: 15 Nov 2020 8:40:22am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class NodeConnectionManagerViewUI;

class NodeManagerViewUI :
    public BaseManagerViewUI<NodeManager, Node, BaseNodeViewUI>,
    public ViewStatsTimer::Listener
{
public:
    NodeManagerViewUI(NodeManager * manager);
    ~NodeManagerViewUI();

    Label statsLabel;
    std::unique_ptr< NodeConnectionManagerViewUI> connectionManagerUI;


    BaseNodeViewUI* createUIForItem(Node * n) override;

    void resized() override;
    void resizedInternalHeader(Rectangle<int>& r) override;

    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void refreshStats() override;

    NodeConnector * getCandidateConnector(bool lookForInput, NodeConnectionType connectionType, BaseNodeViewUI* excludeUI = nullptr);

};


class NodeManagerViewPanel :
    public ShapeShifterContentComponent,
    public Button::Listener,
    public InspectableSelectionManager::AsyncListener
{
public:
    NodeManagerViewPanel(StringRef contentName);
    ~NodeManagerViewPanel();

    std::unique_ptr<NodeManagerViewUI> managerUI;

    OwnedArray<TextButton> crumbsBT;
    Array<NodeManager*> crumbManagers;

    void setManager(NodeManager* manager);
    void updateCrumbs();

    void resized() override;

    void buttonClicked(Button* b) override;

    void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

    static NodeManagerViewPanel* create(const String& name) { return new NodeManagerViewPanel(name); }

};