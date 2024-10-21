/*
  ==============================================================================

	ScreenEditorPanel.h
	Created: 19 Nov 2023 11:18:47am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ScreenEditorPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::Listener,
	public Inspectable::InspectableListener,
	public OpenGLSharedRenderer,
	public DragAndDropTarget,
	public KeyListener,
	public EngineListener
{
public:
	ScreenEditorPanel();
	~ScreenEditorPanel();

	Screen* screen;
	WeakReference<Inspectable> screenRef;

	Rectangle<int> frameBufferRect;
	Point2DParameter* closestHandle;
	Point2DParameter* selectedPinMediaHandle;
	Surface* manipSurface;
	Array<Point<float>> posAtMouseDown;

	Array<Point2DParameter*> overlapHandles;

	bool panningMode;
	Point<float> offsetAtMouseDown;
	Point<float> focusPointAtMouseDown;

	float zoomSensitivity;
	bool zoomingMode;
	float zoom;
	float zoomAtMouseDown;
	Point<float> viewOffset;

	//drag drop
	Surface* candidateDropSurface;

	GLuint framebuffer;

	void setScreen(Screen* s);
	void paint(Graphics& g) override;
	
	Path getSurfacePath(Surface* s);

	void mouseDown(const MouseEvent& e) override;
	void mouseMove(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;


	Point<float> getRelativeMousePos();
	Point<float> getRelativeScreenPos(Point<int> screenPos);
	Point<int> getPointOnScreen(Point<float> pos);
	void moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen);

	bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
	void itemDragEnter(const SourceDetails& source) override;
	void itemDragMove(const SourceDetails& source) override;
	void itemDragExit(const SourceDetails& source) override;
	void itemDropped(const SourceDetails& source) override;

	void setCandidateDropSurface(Surface* s, Media* m = nullptr);

	bool keyPressed(const KeyPress& key, Component* originatingComponent) override;

	// Inherited via OpenGLRenderer
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	void inspectablesSelectionChanged() override;
	void inspectableDestroyed(Inspectable* i) override;

	void startLoadFile() override;

	static ScreenEditorPanel* create(const String& name) { return new ScreenEditorPanel(); }
};