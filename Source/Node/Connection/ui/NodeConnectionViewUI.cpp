
/*
  ==============================================================================

	NodeConnectionViewUI.cpp
	Created: 16 Nov 2020 10:01:02am
	Author:  bkupe

  ==============================================================================
*/

#include "Node/NodeIncludes.h"

NodeConnectionViewUI::NodeConnectionViewUI(NodeConnection* connection, NodeConnector* _sourceConnector, NodeConnector* _destConnector) :
	BaseItemMinimalUI(connection),
	sourceConnector(nullptr),
	destConnector(nullptr),
	prevHasSent(false),
	sourceHandle(connection != nullptr ? NodeConnection::getColorForType(connection->connectionType) : NORMAL_COLOR),
	destHandle(connection != nullptr ? NodeConnection::getColorForType(connection->connectionType) : NORMAL_COLOR)
{
	setOpaque(false);

	dragAndDropEnabled = false;

	setSourceConnector(_sourceConnector);
	setDestConnector(_destConnector);

	setRepaintsOnMouseActivity(true);
	autoDrawContourWhenSelected = false;


	if (connection != nullptr)
	{
		sourceHandle.setSize(16, 16);
		destHandle.setSize(16, 16);
		addAndMakeVisible(&sourceHandle);
		addAndMakeVisible(&destHandle);
	}

	ViewStatsTimer::getInstance()->addListener(this);
}

NodeConnectionViewUI::~NodeConnectionViewUI()
{
	setSourceConnector(nullptr);
	setDestConnector(nullptr);

	if (ViewStatsTimer* vs = ViewStatsTimer::getInstanceWithoutCreating()) vs->removeListener(this);
}

void NodeConnectionViewUI::paint(Graphics& g)
{
	Colour c = NORMAL_COLOR;


	if (item != nullptr && item->enabled->boolValue())
	{
		c = NodeConnection::getColorForType(item->connectionType);
		if (item->isSelected) c = HIGHLIGHT_COLOR;
	}

	bool hasSentInPrevLoop = item != nullptr && item->hasSentInPrevLoop;
	
	if (hasSentInPrevLoop)
	{
		c = c.interpolatedWith(YELLOW_COLOR,.3f);
	}

	if (isMouseOverOrDragging()) c = c.brighter();
	g.setColour(c);
	g.strokePath(path, PathStrokeType(isMouseOverOrDragging() ? 3.0f : hasSentInPrevLoop ? 2.0f : 1.0f));
}

void NodeConnectionViewUI::updateBounds()
{
	if ((sourceConnector == nullptr && destConnector == nullptr) || getParentComponent() == nullptr) return;

	Rectangle<float> mouseR = Rectangle<int>(0, 0, 10, 10).withCentre(getParentComponent()->getMouseXYRelative()).toFloat();
	Rectangle<float> sourceR = sourceConnector == nullptr ? mouseR : getParentComponent()->getLocalArea(sourceConnector, sourceConnector->getLocalBounds().toFloat());
	Rectangle<float> destR = destConnector == nullptr ? mouseR : getParentComponent()->getLocalArea(destConnector, destConnector->getLocalBounds().toFloat());

	Rectangle<int> r = sourceR.getUnion(destR).expanded(20).toNearestInt();
	setBounds(r);
	buildPath();
	resized();
}


void NodeConnectionViewUI::buildPath()
{
	Point<float> mouseP = Rectangle<int>(0, 0, 10, 10).withCentre(getMouseXYRelative()).getCentre().toFloat();

	Point<float> sourceP = sourceConnector == nullptr ? mouseP : getLocalPoint(sourceConnector, sourceConnector->getLocalBounds().getCentre()).toFloat();
	Point<float> destP = destConnector == nullptr ? mouseP : getLocalPoint(destConnector, destConnector->getLocalBounds().getCentre()).toFloat();


	path.clear();
	path.startNewSubPath(sourceP);

	if (destP.x >= sourceP.x - 20)
	{
		Point<float> controlPoint1 = sourceP.translated(60, 0);
		Point<float> controlPoint2 = destP.translated(-60, 0);
		path.cubicTo(controlPoint1, controlPoint2, destP);
	}
	else
	{
		Point<float> controlPoint1 = sourceP.translated(20, 0);
		Point<float> controlPoint2 = destP.translated(-20, 0);
		float middleY = jmax(sourceP.y, destP.y) - 25;
		path.lineTo(controlPoint1);
		path.lineTo(controlPoint1.x, middleY);
		path.lineTo(controlPoint2.x, middleY);
		path.lineTo(controlPoint2);
		path.lineTo(destP);
		path = path.createPathWithRoundedCorners(10);
	}

	hitPath = buildHitPath(&path);

	if (path.getLength() > 60)
	{
		sourceHandle.setVisible(true);
		destHandle.setVisible(true);
		sourceHandle.setCentrePosition(path.getPointAlongPath(15).toInt());
		destHandle.setCentrePosition(path.getPointAlongPath(path.getLength() - 15).toInt());
	}
	else
	{
		sourceHandle.setVisible(false);
		destHandle.setVisible(false);
	}

}
void NodeConnectionViewUI::mouseDoubleClick(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDoubleClick(e);
	if (item != nullptr) item->enabled->setValue(!item->enabled->boolValue());
}

bool NodeConnectionViewUI::hitTest(int x, int y)
{
	return hitPath.contains((float)x, (float)y);
}

void NodeConnectionViewUI::setSourceConnector(NodeConnector* c)
{
	if (sourceConnector == c) return;
	if (sourceConnector != nullptr)
	{
		sourceConnector->removeComponentListener(this);
		if (sourceConnector->getParentComponent() != nullptr) sourceConnector->getParentComponent()->removeComponentListener(this);
	}

	sourceConnector = c;

	if (sourceConnector != nullptr)
	{
		sourceConnector->addComponentListener(this);
		sourceConnector->getParentComponent()->addComponentListener(this);
	}

	updateBounds();
}

void NodeConnectionViewUI::setDestConnector(NodeConnector* c)
{
	if (destConnector == c) return;
	if (destConnector != nullptr)
	{
		destConnector->removeComponentListener(this);
		if (destConnector->getParentComponent() != nullptr) destConnector->getParentComponent()->removeComponentListener(this);
	}

	destConnector = c;

	if (destConnector != nullptr)
	{
		destConnector->addComponentListener(this);
		destConnector->getParentComponent()->addComponentListener(this);
	}

	updateBounds();
}

void NodeConnectionViewUI::componentMovedOrResized(Component& c, bool, bool)
{
	updateBounds();
}

void NodeConnectionViewUI::componentBeingDeleted(Component& c)
{
	if (&c == sourceConnector) setSourceConnector(nullptr);
	else if (&c == destConnector) setDestConnector(nullptr);
}

void NodeConnectionViewUI::refreshStats()
{
	if (item != nullptr && item->hasSentInPrevLoop != prevHasSent)
	{
		repaint();
		prevHasSent = item->hasSentInPrevLoop;
	}
}

Path NodeConnectionViewUI::buildHitPath(Path* sourcePath)
{

	Array<Point<float>> firstPoints;
	Array<Point<float>> secondPoints;

	const int numPoints = 10;
	const int margin = 10;

	Array<Point<float>> points;
	for (int i = 0; i < numPoints; i++)
	{
		points.add(sourcePath->getPointAlongPath(sourcePath->getLength() * i / jmax(numPoints - 1, 1)));
	}

	for (int i = 0; i < numPoints; i++)
	{
		Point<float> tp;
		Point<float> sp;

		if (i == 0 || i == numPoints - 1)
		{
			tp = points[i].translated(0, -margin);
			sp = points[i].translated(0, margin);
		}
		else
		{
			float pi = MathConstants<float>::pi;
			float angle1 = points[i].getAngleToPoint(points[i - 1]);
			float angle2 = points[i].getAngleToPoint(points[i + 1]);

			if (angle1 < 0) angle1 += pi * 2;

			if (angle2 < 0) angle2 += pi * 2;

			float angle = (angle1 + angle2) / 2.f;

			if (angle1 < angle2) angle += pi;

			// DBG("Point " << i << ", angle : " << angle << " >>  " << String(angle1>angle2));

			tp = points[i].getPointOnCircumference(margin, angle + pi);
			sp = points[i].getPointOnCircumference(margin, angle);
		}

		firstPoints.add(tp);
		secondPoints.insert(0, sp);
	}

	Path _hitPath;
	_hitPath.startNewSubPath(firstPoints[0]);
	for (int i = 1; i < firstPoints.size(); i++) _hitPath.lineTo(firstPoints[i]);
	for (int i = 0; i < secondPoints.size(); i++) _hitPath.lineTo(secondPoints[i]);
	_hitPath.closeSubPath();

	return _hitPath;
}




NodeConnectionViewUI::Handle::Handle(Colour c) :
	color(c)
{
	setRepaintsOnMouseActivity(true);
}

void NodeConnectionViewUI::Handle::paint(Graphics& g)
{
	Rectangle<float> r = getLocalBounds().reduced(4).toFloat();
	g.setColour(isMouseOverOrDragging() ? color.brighter(.1f) : BG_COLOR);
	g.fillEllipse(r);

	g.setColour(color.brighter(isMouseOverOrDragging() ? .3f : 0));
	g.drawEllipse(r, 1);
}

