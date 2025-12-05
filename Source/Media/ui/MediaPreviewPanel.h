/*
  ==============================================================================

	MediaPreviewPanel.h
	Created: 28 Aug 2024 11:46:30am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class MediaPreview :
	public Component,
	public OpenGLSharedRenderer,
	public Inspectable::InspectableListener,
	public MediaTarget,
	public KeyListener

{
public:

	MediaPreview();
	virtual ~MediaPreview();
	
	bool useMediaOnPreview;
	Media* media;

	IInteractableMedia* iMedia; //if media is webmedia

	Rectangle<int> mediaRect;

	void setMedia(Media* m);

	void paint(Graphics& g) override;
	void resized() override;

	void computeMediaRect();

	void renderOpenGL() override;
	void openGLContextClosing() override;
	void inspectableDestroyed(Inspectable* i) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
	void mouseMove(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	bool keyPressed(const KeyPress& key, Component* originatingComponent) override;

};

class MediaPreviewPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::AsyncListener,
	public Parameter::AsyncListener
{
	public:

	MediaPreviewPanel();
	virtual ~MediaPreviewPanel();

	MediaPreview preview;
	BoolParameter lockPreview;
	std::unique_ptr<BoolToggleUI> lockPreviewUI;
	

	void paint(Graphics& g) override;
	void resized() override;

	void checkAndAssignPreview(InspectableSelectionManager* selectionManager = nullptr);

	void newMessage(const Parameter::ParameterEvent& e) override;

	void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

	static MediaPreviewPanel* create(const String& name) { return new MediaPreviewPanel(); }
};