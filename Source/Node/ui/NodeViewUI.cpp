/*
  ==============================================================================

	BaseNodeViewUI.cpp
	Created: 15 Nov 2020 9:26:57am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

juce_ImplementSingleton(ViewStatsTimer);

BaseNodeViewUI::BaseNodeViewUI(Node* node) :
	ItemUI(node, Direction::ALL, true)
{
	dragAndDropEnabled = true;
	autoHideWhenDragging = false;
	drawEmptyDragIcon = true;
	showRemoveBT = false;

	statsLabel.setFont(FontOptions(10));
	statsLabel.setColour(statsLabel.textColourId, NORMAL_COLOR.brighter());
	statsLabel.setOpaque(false);
	statsLabel.setJustificationType(Justification::centredRight);
	statsLabel.setInterceptsMouseClicks(false, false);
	addAndMakeVisible(statsLabel);

	updateConnectors();
	item->addAsyncNodeListener(this);

	ViewStatsTimer::getInstance()->addListener(this);

}

BaseNodeViewUI::~BaseNodeViewUI()
{
	if (!inspectable.wasObjectDeleted()) item->removeAsyncNodeListener(this);
	if (ViewStatsTimer* vs = ViewStatsTimer::getInstanceWithoutCreating()) vs->removeListener(this);
}

NodeConnector* BaseNodeViewUI::getConnectorForSlot(NodeConnectionSlot* s)
{
	OwnedArray<NodeConnector>* arr = s->isInput ? &inConnectors : &outConnectors;
	for (auto& c : *arr) if (c->slot == s) return c;
	return nullptr;
}

Array<NodeConnector*> BaseNodeViewUI::getConnectorsForType(NodeConnectionType t, bool isInput)
{
	Array<NodeConnector*> result;

	OwnedArray<NodeConnector>* arr = isInput ? &inConnectors : &outConnectors;
	for (auto& c : *arr) if (c->slot->type == t) result.add(c);

	return result;
}

void BaseNodeViewUI::updateConnectors()
{
	inConnectors.clear();

	for (auto& i : item->inSlots)
	{
		NodeConnector* c = new NodeConnector(i, this);
		inConnectors.add(c);
		addAndMakeVisible(c);
	}

	outConnectors.clear();

	for (auto& i : item->outSlots)
	{
		NodeConnector* c = new NodeConnector(i, this);
		outConnectors.add(c);
		addAndMakeVisible(c);
	}
}

void BaseNodeViewUI::paint(Graphics& g)
{
	ItemUI::paint(g);

	Rectangle<int> mr = getMainBounds();
	mr.removeFromTop(headerHeight + headerGap);

	//if (!item->miniMode->boolValue())
	//{
	//	GenericScopedLock lock(item->imageLock);
	//	Image img = item->getPreviewImage();
	//	if(img.isValid())
	//	{
	//		RectanglePlacement p;
	//		p.getTransformToFit(img.getBounds().toFloat(), mr.toFloat());
	//		g.drawImage(img, mr.toFloat(), p);
	//	}
	//}
}

void BaseNodeViewUI::paintOverChildren(Graphics& g)
{
	ItemUI::paintOverChildren(g);
}

void BaseNodeViewUI::resized()
{
	ItemUI::resized();

	int w = 10;
	Rectangle<int> inR = getLocalBounds().removeFromLeft(w).reduced(0, 10);
	Rectangle<int> outR = getLocalBounds().removeFromRight(w).reduced(0, 10);

	Rectangle<int> inCR = inR.removeFromTop(w);
	for (auto& i : inConnectors)
	{
		i->setBounds(inCR);
		if (!item->miniMode->boolValue())
		{
			inR.removeFromTop(4);
			inCR = inR.removeFromTop(w);
		}
	}

	Rectangle<int> outCR = outR.removeFromTop(w);
	for (auto& i : outConnectors)
	{
		i->setBounds(outCR);
		if (!item->miniMode->boolValue())
		{
			outR.removeFromTop(4);
			outCR = outR.removeFromTop(w);
		}
	}
}

void BaseNodeViewUI::resizedInternalHeader(Rectangle<int>& r)
{
	ItemUI::resizedInternalHeader(r);
	statsLabel.setBounds(r.removeFromRight(80));
}

void BaseNodeViewUI::resizedInternalContent(Rectangle<int>& r)
{
	ItemUI::resizedInternalContent(r);
	resizedInternalContentNode(r);
}

Rectangle<int> BaseNodeViewUI::getMainBounds()
{
	return getLocalBounds().reduced(10, 0);
}

void BaseNodeViewUI::nodeInputsChanged()
{
}

void BaseNodeViewUI::nodeOutputsChanged()
{
}

void BaseNodeViewUI::viewFilterUpdated()
{
	resized();
}

void BaseNodeViewUI::refreshStats()
{
	if (inspectable.wasObjectDeleted()) return;
	statsLabel.setText(String(item->processTimeMS) + "ms", dontSendNotification);
	//if (!item->miniMode->boolValue() && item->getPreviewImage().isValid()) repaint();
}

void BaseNodeViewUI::newMessage(const Node::NodeEvent& e)
{
	if (e.type == e.INPUTS_CHANGED) nodeInputsChanged();
	else if (e.type == e.OUTPUTS_CHANGED) nodeOutputsChanged();
	else if (e.type == e.VIEW_FILTER_UPDATED) viewFilterUpdated();
}
