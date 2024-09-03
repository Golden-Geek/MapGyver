/*
  ==============================================================================

    CompositionEditorPanel.h
    Created: 28 Aug 2024 11:45:45am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#pragma once

//class CompositionEditorView :
//	public InspectableContentComponent,
//	public OpenGLRenderer,
//	public DragAndDropTarget,
//	public KeyListener,
//	public ContainerAsyncListener
//{
//public:
//	CompositionEditorView(Composition* surface);
//	~CompositionEditorView();
//
//	Composition* surface;
//
//	Rectangle<int> frameBufferRect;
//
//	bool panningMode;
//	Point<float> offsetAtMouseDown;
//	Point<float> focusPointAtMouseDown;
//
//	float zoomSensitivity;
//	bool zoomingMode;
//	float zoom;
//	float zoomAtMouseDown;
//	Point<float> viewOffset;
//
//	GLuint framebuffer;
//
//	class FocusComp :
//		public ResizableBorderComponent
//	{
//	public:
//		FocusComp(CompositionEditorView* view);
//		~FocusComp();
//
//		CompositionEditorView* view;
//
//		void paint(Graphics& g) override;
//		void resized() override;
//
//	};
//
//	FocusComp focusComp;
//	bool updatingFocus;
//
//	void resized() override;
//	void visibilityChanged() override;
//
//	void paint(Graphics& g) override;
//	void newOpenGLContextCreated() override;
//	void renderOpenGL() override;
//	void openGLContextClosing() override;
//
//	void mouseDown(const MouseEvent& e) override;
//	void mouseMove(const MouseEvent& e) override;
//	void mouseDrag(const MouseEvent& e) override;
//	void mouseUp(const MouseEvent& e) override;
//	void mouseExit(const MouseEvent& e) override;
//	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
//
//	Point<float> getRelativeMousePos();
//	Point<float> getRelativeScreenPos(Point<int> screenPos);
//	Point<int> getPointOnScreen(Point<float> pos);
//	void moveScreenPointTo(Point<float> screenPos, Point<int> posOnScreen);
//
//
//	bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
//	void itemDropped(const SourceDetails& source) override;
//
//	bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
//
//	void updateFocus();
//	void childBoundsChanged(Component* child) override;
//
//	void newMessage(const ContainerAsyncEvent& e) override;
//};
//
//class CompositionEditorPanel :
//	public ShapeShifterContentComponent,
//	public InspectableSelectionManager::Listener,
//	public Inspectable::InspectableListener
//{
//public:
//	CompositionEditorPanel(const String& name);
//	~CompositionEditorPanel();
//
//	std::unique_ptr<CompositionEditorView> surfaceEditorView;
//
//	void paint(Graphics& g) override;
//	void resized() override;
//
//	void setCurrentComposition(Composition* surface);
//
//	void inspectablesSelectionChanged() override;
//	void inspectableDestroyed(Inspectable* i) override;
//
//
//	static CompositionEditorPanel* create(const String& name) { return new CompositionEditorPanel(name); }
//};