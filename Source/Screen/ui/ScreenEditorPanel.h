/*
  ==============================================================================

	ScreenEditorPanel.h
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ScreenEditorView :
	public InspectableContentComponent,
	public OpenGLRenderer
{
public:
	ScreenEditorView(Screen* screen);
	~ScreenEditorView();

	Screen* screen;
	OpenGLContext context;

	Rectangle<int> frameBufferRect;
	Point2DParameter* closestHandle;
	Surface* manipSurface;
	Array<Point<float>> posAtMouseDown;

	Array<Point2DParameter*> overlapHandles;

	void paint(Graphics& g) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseMove(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;

	Point<float> getRelativeMousePos();
	Point<float> getRelativeScreenPos(Point<int> screenPos);
	Point<int> getPointOnScreen(Point<float> pos);

	// Inherited via OpenGLRenderer
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;
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