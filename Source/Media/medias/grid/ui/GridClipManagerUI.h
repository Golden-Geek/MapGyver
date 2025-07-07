/*
  ==============================================================================

	GridClipManagerUI.h
	Created: 11 Jun 2025 4:44:26pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridClipManagerUI :
	public ManagerUI<GridClipManager, GridClip, GridClipUI>
{
public:
	GridClipManagerUI(GridClipManager* manager);
	~GridClipManagerUI();

	void paint(Graphics& g) override;
};