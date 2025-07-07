/*
  ==============================================================================

	GridLayerManager.h
	Created: 12 Feb 2025 11:50:39am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayerManager :
	public Manager<GridLayer>
{
public:
	GridLayerManager();
	~GridLayerManager() override;
};