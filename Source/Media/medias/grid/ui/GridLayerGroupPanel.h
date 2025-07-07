/*
  ==============================================================================

	GridLayerGroupPanel.h
	Created: 11 Jun 2025 4:32:41pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridLayerGroupPanel : public ItemUI<GridLayerGroup>
{
public:
	GridLayerGroupPanel(GridLayerGroup* item);
	~GridLayerGroupPanel();


	void paint(Graphics& g) override;
	void resized() override;
};
