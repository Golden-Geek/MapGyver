/*
  ==============================================================================

	GridMediaPanel.h
	Created: 12 Feb 2025 11:50:10am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridMediaPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::Listener,
	public Inspectable::InspectableListener
{
public:
	GridMediaPanel(const String& name);
	~GridMediaPanel() override;


	GridMedia* currentMedia;
	std::unique_ptr<GridBoard> gridBoard;

	void paint(Graphics& g) override;
	void resized() override;


	void setGridMedia(GridMedia* media);


	void inspectablesSelectionChanged() override;
	void inspectableDestroyed(Inspectable* inspectable) override;

	static GridMediaPanel* create(const String& name) { return new GridMediaPanel(name); }
};
