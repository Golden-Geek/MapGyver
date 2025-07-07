/*
  ==============================================================================

	GridColumnUI.h
	Created: 11 Jun 2025 4:33:32pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridColumnUI : public ItemMinimalUI<GridColumn>
{
public:
	GridColumnUI(GridColumn* item);
	~GridColumnUI();

	void paint(Graphics& g) override;
	void resized() override;
};
