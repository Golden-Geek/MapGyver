/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"
#include "Surface.h"

Surface::Surface(var params) :
	BaseItem(params.getProperty("name", "Surface")),
	objectType(params.getProperty("type", "Surface").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = true;

	itemDataType = "Surface";

	topLeft = addPoint2DParameter("topLeft ", "");
	topRight = addPoint2DParameter("topRight ", "");
	bottomLeft = addPoint2DParameter("bottomLeft ", "");
	bottomRight = addPoint2DParameter("bottomRight ", "");

	topLeft->setDefaultPoint(0, 1);
	topRight->setDefaultPoint(1, 1);
	bottomLeft->setDefaultPoint(0, 0);
	bottomRight->setDefaultPoint(1, 0);

	topLeft->setBounds(0, 0, 1, 1);
	topRight->setBounds(0, 0, 1, 1);
	bottomLeft->setBounds(0, 0, 1, 1);
	bottomRight->setBounds(0, 0, 1, 1);

	softEdgeTop = addFloatParameter("Soft Edge Top", "", 0, 0, 1);
	softEdgeRight = addFloatParameter("Soft Edge Right", "", 0, 0, 1);
	softEdgeBottom = addFloatParameter("Soft Edge Bottom", "", 0, 0, 1);
	softEdgeLeft = addFloatParameter("Soft Edge Left", "", 0, 0, 1);

	cropTop = addFloatParameter("Crop Top", "", 0, 0, 1);
	cropRight = addFloatParameter("Crop Right", "", 0, 0, 1);
	cropBottom = addFloatParameter("Crop Bottom", "", 0, 0, 1);
	cropLeft = addFloatParameter("Crop Left", "", 0, 0, 1);

	media = addTargetParameter("Media", "Media to read on this screen", MediaManager::getInstance());
	media->maxDefaultSearchLevel = 0;
	media->targetType = TargetParameter::CONTAINER;
}

Surface::~Surface()
{
}

void Surface::onContainerParameterChangedInternal(Parameter* p) 
{
	if (p == topLeft || p == topRight || p == bottomLeft || p == bottomRight)
	{
		updatePath();
	}
}

void Surface::updatePath()
{
	quadPath.clear();
	quadPath.startNewSubPath(topLeft->getPoint());
	quadPath.lineTo(topRight->getPoint());
	quadPath.lineTo(bottomRight->getPoint());
	quadPath.lineTo(bottomLeft->getPoint());
	quadPath.closeSubPath();
}

bool Surface::isPointInside(Point<float> pos)
{
	return quadPath.contains(pos);
}

