/*
  ==============================================================================

    WebMedia.h
    Created: 11 Feb 2025 3:03:12pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include <Ultralight/Ultralight.h>

class WebMedia :
    public ImageMedia,
    public ultralight::LoadListener,
    public ultralight::ViewListener,
    public ultralight::Logger
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

    bool glCleared = false;

    void clearItem() override;

    // Overrides
    void onContainerParameterChangedInternal(Parameter* p) override;
    void onContainerTriggerTriggered(Trigger* t) override;


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

    void EnsureRenderer();

    void LogMessage(ultralight::LogLevel log_level, const ultralight::String& message) override;

    DECLARE_TYPE("Web")

    // Inherited via Logger
};