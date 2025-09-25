/*
  ==============================================================================

	GridLayerPanel.h
	Created: 12 Feb 2025 11:52:08am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayerPanel : public ItemUI<GridLayer>
{
public:
	GridLayerPanel(GridLayer* item);
	~GridLayerPanel();

	void paint(Graphics& g) override;
	void resized() override;
};