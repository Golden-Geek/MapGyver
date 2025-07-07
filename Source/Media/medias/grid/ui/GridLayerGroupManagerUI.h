/*
  ==============================================================================

    GridLayerGroupManagerUI.h
    Created: 12 Feb 2025 11:52:24am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayerGroupManagerUI :
    public ManagerUI<GridLayerGroupManager, GridLayerGroup, GridLayerGroupPanel>
{
public:
    GridLayerGroupManagerUI(GridLayerGroupManager* manager);
    ~GridLayerGroupManagerUI();

};