/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

Screen::Screen(var params) :
	BaseItem(params.getProperty("name", "Screen")),
	objectType(params.getProperty("type", "Screen").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;

	itemDataType = "Screen";

	output.reset(new ScreenOutput(this));

	screenNumber = addIntParameter("Screen number", "Screen ID in your OS", 1, 0);
	//enabled->setDefaultValue(false);

	snapDistance = addFloatParameter("Snap distance", "Distance in pixels to snap to another point", .05f, 0, .2f);

	addChildControllableContainer(&surfaces);

	if (!Engine::mainEngine->isLoadingFile)
	{
		updateOutputLiveStatus();
	}
}

Screen::~Screen()
{
}

void Screen::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled || p == screenNumber)
	{
		updateOutputLiveStatus();
	}
}

void Screen::updateOutputLiveStatus()
{
	if (isCurrentlyLoadingData) return;

	if (enabled->boolValue())
	{
		output->stopLive();
		output->goLive(screenNumber->intValue());
	}
	else
	{
		output->stopLive();
	}
}

Point2DParameter* Screen::getClosestHandle(Point<float> pos, float maxDistance, Array<Point2DParameter*> excludeHandles)
{
	Point2DParameter* result = nullptr;

	float closestDist = maxDistance;
	for (auto& s : surfaces.items)
	{
		Array<Point2DParameter*> handles = { s->topLeft, s->topRight, s->bottomLeft, s->bottomRight };
		for (auto& h : handles)
		{
			if (excludeHandles.contains(h)) continue;
			float dist = h->getPoint().getDistanceFrom(pos);
			if (maxDistance > 0 && dist > maxDistance) continue;
			if (dist < closestDist)
			{
				result = h;
				closestDist = dist;
			}
		}
	}
	return result;
}

Point2DParameter* Screen::getSnapHandle(Point<float> pos, Point2DParameter* handle)
{
	return getClosestHandle(pos, snapDistance->floatValue(), { handle });
}

Array<Point2DParameter*> Screen::getOverlapHandles(Point2DParameter* handle)
{
	Array<Point2DParameter*> result;
	for (auto& s : surfaces.items)
	{
		Array<Point2DParameter*> handles = { s->topLeft, s->topRight, s->bottomLeft, s->bottomRight };
		for (auto& h : handles)
		{
			if (h == handle) continue;
			if (h->getPoint() == handle->getPoint())
				result.add(h);
		}
	}
	return result;
}

void Screen::afterLoadJSONDataInternal()
{
	updateOutputLiveStatus();
}

