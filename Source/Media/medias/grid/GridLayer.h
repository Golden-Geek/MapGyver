/*
  ==============================================================================

	GridLayer.h
	Created: 12 Feb 2025 11:50:28am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayer : 
	public BaseItem
{
public:
	GridLayer(const String& name = "Grid Layer", var params = var());
	virtual ~GridLayer();
	void clearItem() override;


	DECLARE_TYPE("Grid Layer");
};