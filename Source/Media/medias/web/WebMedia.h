/*
  ==============================================================================

	WebMedia.h
	Created: 11 Feb 2025 3:03:12pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include <Ultralight/Ultralight.h>

class WebMedia;


class UltralightManager :
	public ultralight::Logger
{
public:

	juce_DeclareSingleton(UltralightManager, true);

	UltralightManager();
	~UltralightManager();

	Array<WebMedia*, CriticalSection> clients;

	void registerClient(WebMedia* client);
	void unregisterClient(WebMedia* client);

	void setupRenderer();
	void clear();

	void update();

	ultralight::RefPtr<ultralight::Renderer> getRenderer();
	void LogMessage(ultralight::LogLevel log_level, const ultralight::String& message) override;

	ultralight::RefPtr<ultralight::Renderer> renderer;
};

class WebMedia :
	public ImageMedia,
	public IInteractableMedia,
	public ultralight::LoadListener,
	public ultralight::ViewListener
{
public:
	WebMedia(var params = var());
	virtual ~WebMedia();

	// Parameters
	StringParameter* urlParam;
	FloatParameter* zoomParam;
	BoolParameter* transparentParam;
	Trigger* reloadTrigger;

	// Ultralight
	ultralight::RefPtr<ultralight::View> view;
	static ultralight::RefPtr<ultralight::Renderer> ultralightRenderer;

	static bool platformInitialized;

	bool glCleared = false;

	// Overrides
	void onContainerParameterChangedInternal(Parameter* p) override;
	void onContainerTriggerTriggered(Trigger* t) override;

	void setupView();
	void initGLInternal() override;

	void renderOpenGL() override;

	void preRenderGLInternal() override;

	void closeGLInternal() override;

	// Manage resizing
	void initFrameBuffer() override;

	// Ultralight Listeners
	void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
	void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
	void OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) override;
	void OnChangeTitle(ultralight::View* caller, const ultralight::String& title) override;


	//Interactions

	Point<int> getMediaMousePosition(const MouseEvent& e, Rectangle<int> canvasRect);

	void sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
	void sendKeyPressed(const KeyPress& key) override;

	void sendMouseEventToUltralight(const ultralight::MouseEvent& event);
	void sendScrollEventToUltralight(const ultralight::ScrollEvent& event);
	void sendKeyEventToUltralight(const ultralight::KeyEvent& event);
	DECLARE_TYPE("Web")

	// Inherited via Logger
};