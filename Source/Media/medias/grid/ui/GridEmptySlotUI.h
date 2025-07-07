/*
  ==============================================================================

	GridEmptySlotUI.h
	Created: 11 Jun 2025 4:41:03pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridEmptySlotUI : public Component
{
public:
	GridEmptySlotUI();
	~GridEmptySlotUI();

	void paint(Graphics& g) override;
	void resized() override;
};
