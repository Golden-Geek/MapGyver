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


WebMedia::WebMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	// Parameters
	urlParam = addStringParameter("URL", "The website address", "https://goldengeek.org/");
	zoomParam = addFloatParameter("Zoom", "Zoom level", 1.0f, 0.1f, 5.0f);
	transparentParam = addBoolParameter("Transparent", "Transparent Background", false);
	reloadTrigger = addTrigger("Reload", "Reload Page");


	// Use default size from parameters (inherited from Media)
	int w = width != nullptr ? width->intValue() : 1920;
	int h = height != nullptr ? height->intValue() : 1080;
	initImage(w, h);

	UltralightManager::getInstance()->registerClient(this);
}

WebMedia::~WebMedia()
{
	if (UltralightManager::getInstanceWithoutCreating())
		UltralightManager::getInstance()->unregisterClient(this);
}


void WebMedia::setupView()
{

	// Create View specific to THIS instance
	ultralight::ViewConfig viewConfig;
	viewConfig.is_accelerated = false; // CPU rendering
	viewConfig.is_transparent = false;

	auto ultralightRenderer = UltralightManager::getInstance()->getRenderer();
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

void WebMedia::initGLInternal()
{
	UltralightManager::getInstance()->registerClient(this);
	setupView();
}

void WebMedia::onContainerParameterChangedInternal(Parameter* p)
{
	Media::onContainerParameterChangedInternal(p); // Call base

	if (!view) return;

	if (p == urlParam)
	{
		if (!isCurrentlyLoadingData)
		{
			GlContextHolder::getInstance()->callOnGLThread(
				[=]()
				{
					view->LoadURL(urlParam->stringValue().toRawUTF8());
				}
			);
		}
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
	if (!view) return;

	view->set_load_listener(nullptr);
	view->set_view_listener(nullptr);
	view = nullptr;


}

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

UltralightManager::~UltralightManager()
{
	LOG("DESTROY");
	renderer = nullptr;
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
	GlContextHolder::getInstance()->callOnGLThread(
		[=]()
		{
			for (auto& client : clients)
			{
				client->closeGLInternal();
			}

			renderer = nullptr;

		}, true
	);

}

void UltralightManager::update()
{
	if (!renderer || clients.isEmpty()) return;
	renderer->Update();
	renderer->Render();
}

ultralight::RefPtr<ultralight::Renderer> UltralightManager::getRenderer()
{
	jassert(renderer);
	return renderer;
}

void UltralightManager::LogMessage(ultralight::LogLevel log_level, const ultralight::String& message) {
	LOG("From Ultralight : " << String(message.utf8().data()) << " ( " << (int)log_level << " )");
}