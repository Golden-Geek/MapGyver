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
	objectData(params),
	sharedTextureSender(nullptr),
	positionCC("Positionning")
{
	saveAndLoadRecursiveData = true;

	itemDataType = "Screen";

	screenX = positionCC.addIntParameter("Screen X", "Screen X position", 0, 0);
	screenY = positionCC.addIntParameter("Screen Y", "Screen Y position", 0, 0);
	screenWidth = positionCC.addIntParameter("Screen width", "Screen width in pixels", 1920, 0, 10000);
	screenHeight = positionCC.addIntParameter("Screen height", "Screen height in pixels", 1080, 0, 10000);
	positionCC.editorIsCollapsed = true;
	positionCC.enabled->setValue(false);
	addChildControllableContainer(&positionCC);

	outputType = addEnumParameter("Output type", "Output type");
	outputType->addOption("Display", DISPLAY)->addOption("Shared Texture", SHARED_TEXTURE)->addOption("NDI", NDI);

	screenID = addIntParameter("Screen number", "Screen ID in your OS", 1, 0);

	showTestPattern = addBoolParameter("Show Test Pattern", "Show a test pattern on the screen", false);

	snapDistance = addFloatParameter("Snap distance", "Distance in pixels to snap to another point", .05f, 0, .2f);

	if (!Engine::mainEngine->isLoadingFile) surfaces.addItem(nullptr, var(), false);


	addChildControllableContainer(&surfaces);

	renderer.reset(new ScreenRenderer(this));
}

Screen::~Screen()
{

}

void Screen::clearItem()
{
	BaseItem::clearItem();

	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->removeSender(sharedTextureSender);
	sharedTextureSender = nullptr;

	//enabled->setValue(false);
}

void Screen::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == outputType)
	{
		setupOutput();
	}

	if (sharedTextureSender != nullptr)
	{
		if (p == enabled) sharedTextureSender->setEnabled(enabled->boolValue());
		else if (p == screenWidth || p == screenHeight) sharedTextureSender->setSize(screenWidth->intValue(), screenHeight->intValue());
	}
}

void Screen::onContainerNiceNameChanged()
{
	BaseItem::onContainerNiceNameChanged();
	if (sharedTextureSender != nullptr) sharedTextureSender->setSharingName(niceName);
}

void Screen::setupOutput()
{
	SharedTextureManager::getInstance()->removeSender(sharedTextureSender);
	sharedTextureSender = nullptr;

	OutputType type = outputType->getValueDataAsEnum<OutputType>();
	switch (type)
	{
	case DISPLAY:
		break;

	case SHARED_TEXTURE:
		sharedTextureSender = SharedTextureManager::getInstance()->addSender(niceName, screenWidth->intValue(), screenHeight->intValue());
		sharedTextureSender->setExternalFBO(&renderer->frameBuffer);
		break;

	case NDI:
		break;
	}
}

Point2DParameter* Screen::getClosestHandle(Point<float> pos, float maxDistance, Array<Point2DParameter*> excludeHandles)
{
	Point2DParameter* result = nullptr;

	float closestDist = maxDistance;
	for (auto& s : surfaces.items)
	{
		if (!s->enabled->boolValue() || s->isUILocked->boolValue()) continue;

		Array<Point2DParameter*> handles = { s->topLeft, s->topRight, s->bottomLeft, s->bottomRight };


		if (s->bezierCC.enabled->boolValue()) {
			Array<Point2DParameter*> bezierHandles = { s->handleBezierTopLeft, s->handleBezierTopRight, s->handleBezierBottomLeft, s->handleBezierBottomRight, s->handleBezierLeftTop, s->handleBezierLeftBottom, s->handleBezierRightTop, s->handleBezierRightBottom };
			handles.addArray(bezierHandles);
		}

		for (int i = 0; i < s->pinsCC.items.size(); i++) {
			handles.add(s->pinsCC.items[i]->position);
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
		if (!s->enabled->boolValue() || s->isUILocked->boolValue()) continue;
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
		if (!s->enabled->boolValue() || s->isUILocked->boolValue()) continue;
		if (s->isPointInside(pos)) return s;
	}
	return nullptr;
}
