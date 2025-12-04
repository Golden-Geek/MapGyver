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

uint32 WebMedia::lastUpdateTime = 0;




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

	UltralightManager::getInstance()->registerClient(this);
}

WebMedia::~WebMedia()
{
	UltralightManager::getInstance()->unregisterClient(this);
}


void WebMedia::initGLInternal()
{
	UltralightManager::getInstance()->registerClient(this);

	if (!image.isValid())
	{
		LOGERROR("WebMedia::initGLInternal: Image is not valid!");
		return;
	}


	// Create View specific to THIS instance
	ultralight::ViewConfig viewConfig;
	viewConfig.is_accelerated = false; // CPU rendering
	viewConfig.is_transparent = false;

	auto ultralightRenderer = UltralightManager::getInstance()->renderer;
	if (ultralightRenderer)
	{
		// Re-create view if it doesn't exist
		if (!view)
		{
			view = ultralightRenderer->CreateView(image.getWidth(), image.getHeight(), viewConfig, nullptr);
			if (view)
			{
				view->set_load_listener(this);
				view->set_view_listener(this);
				view->LoadURL(urlParam->stringValue().toRawUTF8());
			}
		}
	}
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
		// Media base class handles parameter storage
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


void WebMedia::renderOpenGL()
{
	if (isClearing || isCurrentlyLoadingData) return;

	if (!view)
	{
		initGLInternal();
		return;
	}

	ImageMedia::renderOpenGL();
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

	// 2. Sync Pixels from Ultralight -> ImageMedia::image
	if (!view) return;

	ultralight::BitmapSurface* surface = (ultralight::BitmapSurface*)view->surface();
	if (surface && surface->dirty_bounds().IsValid())
	{
		ultralight::RefPtr<ultralight::Bitmap> bitmap = surface->bitmap();

		// Lock ImageMedia's critical section (inherited)
		GenericScopedLock lock(imageLock);

		void* rawPixels = bitmap->LockPixels();

		// Ensure sizes match before copying
		if (image.getWidth() == (int)bitmap->width() && image.getHeight() == (int)bitmap->height())
		{
			// Copy logic: Ultralight BGRA -> JUCE ARGB
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


	// 3. Call ImageMedia to upload 'image' to 'imageFBO'
	ImageMedia::preRenderGLInternal();
}

void WebMedia::closeGLInternal()
{
	if (view)
	{
		view->set_load_listener(nullptr);
		view->set_view_listener(nullptr);
		view = nullptr;
	}

	//instanceCount--;

	//// If this was the last web media, release the global renderer.
	//// This ensures that if we create a new one later, we get a fresh Renderer/Platform setup.
	//if (instanceCount <= 0)
	//{
	//    instanceCount = 0; // safety
	//    ultralightRenderer = nullptr;
	//}

	// Note: We DO NOT destroy ultralightRenderer here. 
	// It is destroyed in the Destructor when instanceCount hits 0.
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




//MANAGER

juce_ImplementSingleton(UltralightManager)

UltralightManager::UltralightManager() {
	setupRenderer();
}

void UltralightManager::registerClient(WebMedia* client) {
	clients.addIfNotAlreadyThere(client);
}

void UltralightManager::unregisterClient(WebMedia* client) {
	clients.removeFirstMatchingValue(client);
}

void UltralightManager::setupRenderer() {
	LOG("Init Ultralight Platform");
	ultralight::Config config;

	File f = File::getSpecialLocation(File::currentExecutableFile);
#if JUCE_MAC
	f = f.getParentDirectory().getChildFile("Resources");
#endif
	String path = f.getParentDirectory().getFullPathName().toStdString();

	ultralight::Platform::instance().set_config(config);
	ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());

	ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem(path.toRawUTF8()));
	ultralight::Platform::instance().set_logger(this);

	LOG("Init Ultralight Renderer");
	renderer = ultralight::Renderer::Create();
}

void UltralightManager::clear()
{
	renderer = nullptr;
}

void UltralightManager::update()
{
	if (!renderer || clients.isEmpty()) return;
	renderer->Update();
	renderer->Render();
}

void UltralightManager::LogMessage(ultralight::LogLevel log_level, const ultralight::String& message) {
	LOG("From Ultralight : " << String(message.utf8().data()) << " ( " << (int)log_level << " )");
}