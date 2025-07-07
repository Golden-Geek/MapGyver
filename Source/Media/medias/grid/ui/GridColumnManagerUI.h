/*
  ==============================================================================

    GridColumnManagerUI.h
    Created: 11 Jun 2025 4:38:30pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridColumnManagerUI :
    public ManagerUI<GridColumnManager, GridColumn, GridColumnUI>
{
public:
    GridColumnManagerUI(GridColumnManager* manager);
    ~GridColumnManagerUI() override;
};