/*
  ==============================================================================

	GridBoard.h
	Created: 12 Feb 2025 11:52:03am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridBoard :
	public Component
{
public:
	GridBoard(GridMedia* gridMedia);
	~GridBoard();

	GridMedia* gridMedia;
	WeakReference<Inspectable> inspectable;

	std::unique_ptr<GridColumnManagerUI> columnManagerUI;
	//std::unique_ptr<GridLayerManagerUI> layerGroupManagerUI;

	void paint(Graphics& g) override;
	void resized() override;

};