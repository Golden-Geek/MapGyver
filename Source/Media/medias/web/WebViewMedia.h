/*
  ==============================================================================

    WebViewMedia.h
    Created: 06 Dec 2025

  ==============================================================================
*/

#pragma once

// WebView2 & Windows Includes
#include <wrl.h>
#include <wil/com.h> // Recommended for WebView2, or use raw COM pointers
#include "WebView2.h"
#include "WebView2Experimental.h" // Crucial for TextureStream
#include <d3d11.h>

// OpenGL Interop Extensions (Windows specific)
typedef HANDLE(WINAPI* PFNWGLDXOPENDEVICENVPROC) (void* dxDevice);
typedef BOOL(WINAPI* PFNWGLDXCLOSEDEVICENVPROC) (HANDLE hDevice);
typedef HANDLE(WINAPI* PFNWGLDXREGISTEROBJECTNVPROC) (HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
typedef BOOL(WINAPI* PFNWGLDXUNREGISTEROBJECTNVPROC) (HANDLE hDevice, HANDLE hObject);
typedef BOOL(WINAPI* PFNWGLDXLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE* hObjects);
typedef BOOL(WINAPI* PFNWGLDXUNLOCKOBJECTSNVPROC) (HANDLE hDevice, GLint count, HANDLE* hObjects);

using namespace Microsoft::WRL;

class WebViewMedia :
    public Media,
    public IInteractableMedia
{
public:
    WebViewMedia(var params = var());
    virtual ~WebViewMedia();


    enum SourceType { Source_File, Source_URL };
    EnumParameter* source;
    FileParameter* filePath;

    // Parameters
    StringParameter* urlParam;
    Trigger* reloadTrigger;
    Trigger* devToolsTrigger;

    // WebView2 Core Objects
    wil::com_ptr<ICoreWebView2CompositionController> compositionController;
    wil::com_ptr<ICoreWebView2Controller> controller;
    wil::com_ptr<ICoreWebView2> webviewWindow;
    wil::com_ptr<ICoreWebView2ExperimentalTextureStream> textureStream;

    // D3D11 & Interop
    wil::com_ptr<ID3D11Device> d3dDevice;
    wil::com_ptr<ID3D11Texture2D> currentD3DTexture;
    HANDLE glDxDeviceHandle = nullptr;
    HANDLE glDxSharedTextureHandle = nullptr;
    GLuint glTextureID = 0;

    // State
    std::atomic<bool> isWebViewReady{ false };
    std::atomic<bool> newFrameAvailable{ false };
    CriticalSection textureLock;

    wil::com_ptr<ID3D11Texture2D> intermediateTexture;
    HANDLE glIntermediateHandle = nullptr; // The handle for the COPY, not the original

	MediaInteractionContainer interactionCC;

    // Overrides
    void onContainerParameterChangedInternal(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;
    void onContainerTriggerTriggered(Trigger* t) override;

    void initGLInternal() override;
    void renderGLInternal() override;
    void closeGLInternal() override;

    void loadURLOrFile();

    Point<int> getDefaultMediaSize() override;

    // Initialization Sequence
    void initWebView();
    void initTextureStream();
    bool initD3D11();
    void setupGLExtensions();

    // Interaction (IInteractableMedia)
    void sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect) override;
    void sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect) override;
    void sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect) override;
    void sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect) override;
    void sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    void sendKeyPressed(const KeyPress& key) override;

    // Helpers
    Point<int> getMediaMousePosition(const MouseEvent& e, Rectangle<int> canvasRect);

    DECLARE_TYPE("WebGL")
};