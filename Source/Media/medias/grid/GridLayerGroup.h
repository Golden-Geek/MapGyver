/*
  ==============================================================================

    GridLayerGroup.h
    Created: 11 Jun 2025 4:32:15pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayerGroup :
    public BaseItem
{
    public:
    GridLayerGroup(var params = var());
    virtual ~GridLayerGroup();
    void clearItem() override;
    GridLayerManager layerManager;

	DECLARE_TYPE("Grid Layer Group");
};