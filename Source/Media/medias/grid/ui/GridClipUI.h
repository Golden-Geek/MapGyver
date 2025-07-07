/*
  ==============================================================================

	GridClipUI.h
	Created: 11 Jun 2025 4:40:04pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridClipUI : public ItemMinimalUI<GridClip>
{
public:
	GridClipUI(GridClip* item);
	~GridClipUI();

	void paint(Graphics& g) override;
	void resized() override;
};
