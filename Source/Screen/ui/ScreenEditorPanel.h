/*
  ==============================================================================

	ScreenEditorPanel.h
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ScreenEditorView :
	public InspectableContentComponent
{
public:
	ScreenEditorView(Screen* screen);
	~ScreenEditorView();

	Screen* screen;

	void paint(Graphics& g) override;
};

class ScreenEditorPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::Listener,
	public Inspectable::InspectableListener
{
public:
	ScreenEditorPanel(const String& name);
	~ScreenEditorPanel();

	std::unique_ptr<ScreenEditorView> screenEditorView;

	void paint(Graphics& g) override;
	void resized() override;

	void setCurrentScreen(Screen* screen);

	void inspectablesSelectionChanged() override;
	void inspectableDestroyed(Inspectable* i) override;

	static ScreenEditorPanel* create(const String& name) { return new ScreenEditorPanel(name); }
};