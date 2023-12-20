/*
  ==============================================================================

    NodeConnector.h
    Created: 16 Nov 2020 3:03:26pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseNodeViewUI;

class NodeConnector :
    public Component,
    public SettableTooltipClient
{
public:
    NodeConnector(NodeConnectionSlot * slot, BaseNodeViewUI * nodeViewUI);
    ~NodeConnector();

    BaseNodeViewUI* nodeViewUI;
    NodeConnectionSlot* slot;
    bool isInput;

    void paint(Graphics& g) override;
};