/*
  ==============================================================================

    GridLayerGroup.cpp
    Created: 11 Jun 2025 4:32:15pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "GridLayerGroup.h"

GridLayerGroup::GridLayerGroup(var params) :
    BaseItem(getTypeString())
{
    saveAndLoadRecursiveData = true;
	addChildControllableContainer(&layerManager);
}

GridLayerGroup::~GridLayerGroup()
{
}

void GridLayerGroup::clearItem()
{
}
