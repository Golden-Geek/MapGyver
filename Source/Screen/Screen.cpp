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

	screenWidth = addIntParameter("Screen width", "Screen width in pixels", 1920, 0, 10000);
	screenHeight = addIntParameter("Screen height", "Screen height in pixels", 1080, 0, 10000);

	outputType = addEnumParameter("Output type", "Output type");
	outputType->addOption("Display", DISPLAY)->addOption("Shared Texture", SHARED_TEXTURE)->addOption("NDI", NDI);

	screenID = addIntParameter("Screen number", "Screen ID in your OS", 1, 0);

	showTestPattern = addBoolParameter("Show Test Pattern", "Show a test pattern on the screen", false);

	snapDistance = addFloatParameter("Snap distance", "Distance in pixels to snap to another point", .05f, 0, .2f);

	if (!Engine::mainEngine->isLoadingFile) surfaces.addItem();

	addChildControllableContainer(&surfaces);

	renderer.reset(new ScreenRenderer(this));
}

Screen::~Screen()
{
}

void Screen::clearItem()
{
	BaseItem::clearItem();
	//enabled->setValue(false);
}

Point2DParameter* Screen::getClosestHandle(Point<float> pos, float maxDistance, Array<Point2DParameter*> excludeHandles)
{
	Point2DParameter* result = nullptr;

	float closestDist = maxDistance;
	for (auto& s : surfaces.items)
	{
		if (!s->enabled->boolValue()) continue;

		Array<Point2DParameter*> handles = { s->topLeft, s->topRight, s->bottomLeft, s->bottomRight };
		Array<Point2DParameter*> bezierHandles = { s->handleBezierTopLeft, s->handleBezierTopRight, s->handleBezierBottomLeft, s->handleBezierBottomRight, s->handleBezierLeftTop, s->handleBezierLeftBottom, s->handleBezierRightTop, s->handleBezierRightBottom };
		if (s->isBezier->boolValue()) {
			handles.addArray(bezierHandles);
		}
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
		if (!s->enabled->boolValue()) continue;
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

Surface* Screen::getSurfaceAt(Point<float> pos)
{
	for (auto& s : surfaces.items)
	{
		if (!s->enabled->boolValue()) continue;
		if (s->isPointInside(pos)) return s;
	}
	return nullptr;
}