/*
  ==============================================================================

    WebMedia.cpp
    Created: 11 Feb 2025 3:03:12pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include <Ultralight/platform/Platform.h>
#include <Ultralight/Renderer.h>
#include <AppCore/Platform.h>
#include "WebMedia.h"


// Static Global Renderer
static ultralight::RefPtr<ultralight::Renderer> globalRenderer = nullptr;

WebMedia::WebMedia(var params) :
    ImageMedia(getTypeString(), params)
{
    // Parameters
    urlParam = addStringParameter("URL", "The website address", "https://ultralig.ht/");
    zoomParam = addFloatParameter("Zoom", "Zoom level", 1.0f, 0.1f, 5.0f);
    transparentParam = addBoolParameter("Transparent", "Transparent Background", false);
    reloadTrigger = addTrigger("Reload", "Reload Page");

   

    // Use default size from parameters (inherited from Media)
    int w = width != nullptr ? width->intValue() : 1920;
    int h = height != nullptr ? height->intValue() : 1080;

  
    // Initialize the base ImageMedia container
    initImage(w, h);
}

WebMedia::~WebMedia()
{
    if (view)
    {
        view->set_load_listener(nullptr);
        view->set_view_listener(nullptr);
        view = nullptr;
    }
}

void WebMedia::EnsureRenderer()
{
    if (globalRenderer) return;

    ultralight::Config config;

    File f = File::getSpecialLocation(File::currentExecutableFile);
    String path = f.getParentDirectory().getFullPathName().toStdString();
    ultralight::Platform::instance().set_config(config);
    ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());
    ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem(path.toRawUTF8()));
    ultralight::Platform::instance().set_logger(this);

    globalRenderer = ultralight::Renderer::Create();

    // Create View
    ultralight::ViewConfig viewConfig;
    viewConfig.is_accelerated = false; // CPU rendering
    viewConfig.is_transparent = false;
    if (globalRenderer)
    {
        view = globalRenderer->CreateView(image.getWidth(), image.getHeight(), viewConfig, nullptr);
        if (view)
        {
            view->set_load_listener(this);
            view->set_view_listener(this);
            view->LoadURL(urlParam->stringValue().toRawUTF8());
        }
    }
}


void WebMedia::LogMessage(ultralight::LogLevel log_level, const ultralight::String& message)
{
	LOG("From Ultralight : " << String(message.utf8().data()) << " ( " << (int)log_level << " )" );
}

void WebMedia::onContainerParameterChangedInternal(Parameter* p)
{
    Media::onContainerParameterChangedInternal(p); // Call base

    if (!view) return;

    if (p == urlParam)
    {
        view->LoadURL(urlParam->stringValue().toRawUTF8());
    }
    else if (p == zoomParam)
    {
        //view->set_zoom(zoomParam->floatValue());
    }
    else if (p == transparentParam)
    {
        //view->set_is_transparent(transparentParam->boolValue());
    }
    else if (p == width || p == height)
    {
        // Media base class handles the parameter storage, we just need to react
        // initFrameBuffer() is usually called by Media when size changes, so we handle resize there
    }
}

void WebMedia::onContainerTriggerTriggered(Trigger* t)
{
    Media::onContainerTriggerTriggered(t);
    if (t == reloadTrigger && view)
    {
        view->Reload();
    }
}

void WebMedia::initGLInternal()
{
    EnsureRenderer();

}

void WebMedia::initFrameBuffer()
{
    // Call base to setup the main FrameBuffer
    ImageMedia::initFrameBuffer();

    // Resize Ultralight View
    if (view)
    {
        view->Resize(frameBuffer.getWidth(), frameBuffer.getHeight());
    }

    // Resize the CPU Image container in ImageMedia
    initImage(frameBuffer.getWidth(), frameBuffer.getHeight());
}

void WebMedia::preRenderGLInternal()
{
    // 1. Tick Ultralight
    if (globalRenderer)
    {
        globalRenderer->Update();
        globalRenderer->Render();
    }

    // 2. Sync Pixels from Ultralight -> ImageMedia::image
    if (view)
    {
        ultralight::BitmapSurface* surface = (ultralight::BitmapSurface*)view->surface();
        if (surface && surface->dirty_bounds().IsValid())
        {
            ultralight::RefPtr<ultralight::Bitmap> bitmap = surface->bitmap();

            // Lock ImageMedia's critical section (inherited)
            GenericScopedLock lock(imageLock);

            void* rawPixels = bitmap->LockPixels();

            // Ensure sizes match before copying
            if (image.getWidth() == bitmap->width() && image.getHeight() == bitmap->height())
            {
                // Get write pointer to JUCE Image

                // Copy logic: Ultralight BGRA -> JUCE ARGB (often compatible layout in memory)
                // We use the stride (rowBytes) to copy safely row by row
                const int bytesPerRow = bitmap->row_bytes();
                const int height = bitmap->height();

                for (int y = 0; y < height; ++y)
                {
                    uint8* srcRow = (uint8*)rawPixels + (y * bytesPerRow);
                    uint8* destRow = bitmapData->getLinePointer(y);
                    memcpy(destRow, srcRow, bytesPerRow);
                }
            }

            bitmap->UnlockPixels();
            surface->ClearDirtyBounds();
        }
    }

    // 3. Call ImageMedia to upload 'image' to 'imageFBO'
    // This handles the GL context locking, texture binding, and glTexSubImage2D
    ImageMedia::preRenderGLInternal();
}

// -- Listeners --

void WebMedia::OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
{
    shouldRedraw = true;
}

void WebMedia::OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
{
    shouldRedraw = true;
}

void WebMedia::OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) {}
void WebMedia::OnChangeTitle(ultralight::View* caller, const ultralight::String& title) {}