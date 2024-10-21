/*
  ==============================================================================

	MediaPreviewPanel.h
	Created: 28 Aug 2024 11:46:30am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaPreview :
	public Component,
	public OpenGLSharedRenderer,
	public Inspectable::InspectableListener,
	public MediaTarget

{
public:

	MediaPreview();
	virtual ~MediaPreview();
	
	bool useMediaOnPreview;
	Media* media;
	Image image;

	void setMedia(Media* m);

	void paint(Graphics& g) override;

	void renderOpenGL() override;
	void openGLContextClosing() override;
	void inspectableDestroyed(Inspectable* i) override;

};

class MediaPreviewPanel :
	public ShapeShifterContentComponent,
	public InspectableSelectionManager::AsyncListener
{
	public:

	MediaPreviewPanel();
	virtual ~MediaPreviewPanel();

	MediaPreview preview;

	void paint(Graphics& g) override;
	void resized() override;

	void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

	static MediaPreviewPanel* create(const String& name) { return new MediaPreviewPanel(); }
};