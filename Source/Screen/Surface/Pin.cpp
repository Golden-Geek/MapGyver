/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

#define SURFACE_TARGET_MEDIA_ID 0
#define SURFACE_TARGET_MASK_ID 1

Pin::Pin(var params) :
	BaseItem(params.getProperty("name", "Pin")),
	objectType(params.getProperty("type", "Pin").toString()),
	objectData(params)
{
	canBeDisabled = true;
	itemDataType = "Pin";

	position = addPoint2DParameter("Position", "");
	position->setBounds(-1, -1, 2, 2);
	mediaPos = addPoint2DParameter("Media position", "");
	mediaPos->setBounds(0, 0, 1, 1);
	ponderation = addFloatParameter("Ponderation", "", 1, 0);
}

Pin::~Pin()
{
}

