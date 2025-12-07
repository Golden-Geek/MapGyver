/*
  ==============================================================================

	WebViewMedia.cpp
	Created: 06 Dec 2025

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include <WebView2EnvironmentOptions.h>
#include "WebViewMedia.h"
#include <comdef.h> // Required for _com_error
#include <d3d11_4.h>
#ifndef WGL_NV_DX_interop
#define WGL_NV_DX_interop 1
#define WGL_ACCESS_READ_ONLY_NV     0x00000000
#define WGL_ACCESS_READ_WRITE_NV    0x00000001
#define WGL_ACCESS_WRITE_DISCARD_NV 0x00000002
#endif

// Function Pointers for NV_DX_interop
static PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = nullptr;
static PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = nullptr;
static PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = nullptr;
static PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = nullptr;
static PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = nullptr;
static PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = nullptr;


void* get_gl_proc_address(const char* name) {
#if JUCE_WINDOWS
	static HMODULE glModule = GetModuleHandleA("opengl32.dll");
	using wglProc = void* (__stdcall*)(const char*);
	static wglProc wgl_get_proc_address = (wglProc)GetProcAddress(glModule, "wglGetProcAddress");

	void* p = nullptr;
	if (wgl_get_proc_address) p = wgl_get_proc_address(name);
	if (p == nullptr || p == (void*)0x1 || p == (void*)0x2 || p == (void*)0x3 || p == (void*)-1) {
		p = (void*)GetProcAddress(glModule, name);
	}
	return p;
#else
	return nullptr;
#endif
}

String getErrorMessage(HRESULT hr)
{
	_com_error err(hr);
	String errorMsg = String(err.ErrorMessage());
	return "Error " + String::toHexString((int)hr) + ": " + errorMsg;
}

WebViewMedia::WebViewMedia(var params) :
	Media(getTypeString(), params)
{
	source = addEnumParameter("Source", "Source");
	source->addOption("URL", Source_URL)->addOption("File", Source_File);

	filePath = addFileParameter("File path", "File path", "");
	filePath->setAutoReload(true);

	// 1. Setup Parameters
	//urlParam = addStringParameter("URL", "Address", "https://webglsamples.org/blob/blob.html");
	urlParam = addStringParameter("URL", "Address", "https://www.goldengeek.org/mapgyver/simple.html");

	reloadTrigger = addTrigger("Reload", "Reload Page");
	devToolsTrigger = addTrigger("DevTools", "Open DevTools");

	// Standard media init
	width = addIntParameter("Width", "Width", 1920, 1, 10000);
	height = addIntParameter("Height", "Height", 1080, 1, 10000);

	// We do NOT call GlContextHolder register here, we wait for initGLInternal
	// But initWebView needs to happen on the Message Thread (Main Thread)
	// We defer it slightly to ensure the component is alive
	juce::MessageManager::callAsync([this]() { initWebView(); });

	alwaysRedraw = true;
}

WebViewMedia::~WebViewMedia()
{
	closeGLInternal();

	// WebView2 objects must be released on the main thread usually
	// Using wil::com_ptr helps, but explicit clearing is safer for multithreading
	if (textureStream) textureStream->Stop();
	textureStream.reset();
	controller.reset();
	webviewWindow.reset();
	d3dDevice.reset();
}

// ==============================================================================
// INITIALIZATION
// ==============================================================================

void WebViewMedia::initWebView()
{
	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS",
		L"--enable-features=msWebView2TextureStream "
		L"--disable-gpu-sandbox "
		L"--enable-gpu-rasterization"
		L"--disable-features=UseSkiaRenderer "
		// NEW FLAGS:
		L"--autoplay-policy=no-user-gesture-required "
		L"--disable-web-security" // Helps with CORS/Permissions in dev
		L"--use-fake-ui-for-media-stream"
	);

	// --- 1. ENSURE COM IS STA ---
	// WebView2 requires Single Threaded Apartment (STA). 
	// JUCE usually sets this, but we force check to be safe.
	APTTYPE aptType;
	APTTYPEQUALIFIER aptQual;
	HRESULT hrCom = CoGetApartmentType(&aptType, &aptQual);
	if (FAILED(hrCom) || (aptType != APTTYPE_STA && aptType != APTTYPE_MAINSTA))
	{
		LOG("COM not in STA mode. Initializing...");
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	}

	// --- 2. INIT D3D11 ---
	if (!initD3D11()) {
		LOGERROR("Cannot initialize WebView: D3D11 subsystem failed.");
		return;
	}

	// --- 3. BROWSER OPTIONS ---
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AdditionalBrowserArguments(
		L"--enable-features=msWebView2TextureStream "

		L"--disable-gpu-sandbox "
		L"--enable-gpu-rasterization"
		L"--disable-features=UseSkiaRenderer "
		// NEW FLAGS:
		L"--autoplay-policy=no-user-gesture-required "
		L"--disable-web-security"
		L"--use-fake-ui-for-media-stream"
		L"--enable-gpu --ignore-gpu-blocklist --enable-webgl --use-angle=default"
	);


	// --- 4. LOCATE EDGE CANARY ---
	File canaryFolder = File::getSpecialLocation(File::windowsLocalAppData)
		.getChildFile("Microsoft/Edge SxS/Application");

	std::wstring browserPathStr;
	LPCWSTR browserPath = nullptr;

	if (canaryFolder.exists() && canaryFolder.isDirectory())
	{
		auto children = canaryFolder.findChildFiles(File::findDirectories, false);
		for (const auto& child : children)
		{
			if (child.getFileName().contains(".")) {
				browserPathStr = child.getFullPathName().toWideCharPointer();
				browserPath = browserPathStr.c_str();
				LOG("Targeting Edge Canary Version: " + child.getFileName());
				break;
			}
		}
	}

	if (!browserPath) {
		LOGERROR("CRITICAL: Edge Canary version folder not found.");
		return;
	}

	// --- 5. USER DATA FOLDER ---
	String uniqueName = "MyWebView2_Data_" + String(Time::getMillisecondCounter());
	File userDataFolder = File::getSpecialLocation(File::tempDirectory).getChildFile(uniqueName);
	userDataFolder.createDirectory();
	std::wstring userDataPath = userDataFolder.getFullPathName().toWideCharPointer();

	// 3. FORCE CAST to IUnknown or ICoreWebView2EnvironmentOptions
// Some versions of the loader fail if you pass the smart pointer directly without explicit casting
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
		browserPath,
		userDataPath.c_str(),
		(ICoreWebView2EnvironmentOptions*)options.Get(), // EXPLICIT CAST HERE
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
			{
				if (FAILED(result)) {
					LOGERROR("Failed to create environment: " + getErrorMessage(result));
					return result;
				}

				// Query for Experimental Interface (Used later for TextureStream)
				auto experimentalEnv = wil::try_com_query<ICoreWebView2ExperimentalEnvironment12>(env);
				if (!experimentalEnv) {
					LOGERROR("ERROR: Could not query Experimental Environment.");
					return E_NOINTERFACE;
				}

				// Query for Composition Interface (Environment3)
				wil::com_ptr<ICoreWebView2Environment3> env3;
				env->QueryInterface(IID_PPV_ARGS(&env3));
				if (!env3) return E_NOINTERFACE;

				HWND hWnd = GetActiveWindow();
				if (!hWnd) {
					LOGERROR("ERROR: No Active Window found!");
					return E_FAIL;
				}

				// --- 7. CREATE CONTROLLER (CRITICAL: Do this BEFORE TextureStream) ---
				HRESULT hrController = env3->CreateCoreWebView2CompositionController(hWnd,
					Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
						[this, experimentalEnv](HRESULT result, ICoreWebView2CompositionController* compController) -> HRESULT
						{
							if (FAILED(result)) {
								LOGERROR("Controller Creation Failed: " + getErrorMessage(result));
								return result;
							}
							LOG("Controller Created Successfully.");

							// 1. Store Controllers
							compositionController = compController;
							compositionController->QueryInterface(IID_PPV_ARGS(&controller));
							controller->get_CoreWebView2(&webviewWindow);

							// 2. Setup Settings & Bounds
							wil::com_ptr<ICoreWebView2Settings> settings;
							webviewWindow->get_Settings(&settings);
							settings->put_IsScriptEnabled(TRUE);
							settings->put_AreDefaultScriptDialogsEnabled(TRUE);

							RECT bounds = { 0, 0, width->intValue(), height->intValue() };
							controller->put_Bounds(bounds);

							controller->put_IsVisible(TRUE);

							webviewWindow->add_PermissionRequested(
								Callback<ICoreWebView2PermissionRequestedEventHandler>(
									[](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT
									{
										COREWEBVIEW2_PERMISSION_KIND kind;
										args->get_PermissionKind(&kind);
										// Log this to confirm the browser is asking
										LOG("Permission Requested: " + String((int)kind));

										args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
										return S_OK;
									}).Get(), nullptr);

							String streamId = "webview-stream-" + String(juce::Time::currentTimeMillis());
							std::wstring streamIdW = streamId.toWideCharPointer();


							String setupScript =
								"if(!window.connectToJuce) { "
								"window.connectToJuce = async function(streamId) {"
								"    console.log('JS: Searching for canvas...');"
								//"    const canvas = document.querySelector('canvas');" // Finds YOUR canvas
								//"    if (!canvas) { console.error('JS: No canvas found on page!'); return; }"

								"const canvas = document.getElementById('unity-canvas') || document.querySelector('#unity-canvas, canvas');"
								" console.log('JS: Selected canvas:', canvas, canvas&& canvas.id, canvas&& canvas.width, canvas&& canvas.height);"

								"    if (window.chrome.webview.registerTextureStream) {"
								"        try {"
								//           1. FAKE A CLICK (Bypasses 'User Gesture' requirement)
								"            document.body.dispatchEvent(new MouseEvent('click', {bubbles: true}));"

								"            const stream = canvas.captureStream(60);"
								"            const track = stream.getVideoTracks()[0];"

								"            await window.chrome.webview.registerTextureStream(streamId, track);"
								"            console.log('JS: SUCCESS - Stream Connected to ' + streamId);"
								"        } catch (e) {"
								"            console.error('JS: Connection Failed:', e.name, e.message);"
								"        }"
								"    }"
								"};"
								"}";

							webviewWindow->AddScriptToExecuteOnDocumentCreated(setupScript.toWideCharPointer(), nullptr);



							EventRegistrationToken token;
							webviewWindow->add_NavigationCompleted(
								Callback<ICoreWebView2NavigationCompletedEventHandler>(
									[this, streamId](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
									{
										BOOL success;
										args->get_IsSuccess(&success);
										if (!success) LOGERROR("Navigation Failed");

										LOG("Connecting JS to Texture Stream with ID: " + streamId);





										//String url = "https://www.goldengeek.org"; // Or use your variable
										//String jsonArgs =
										//	"{ \"origin\": \"" + url + "\", "
										//	"  \"permissions\": [\"videoCapture\", \"sensors\", \"clipboardRead\", \"clipboardWrite\"] }";

										// 3. EXECUTE "Browser.grantPermissions"
										//    This overrides any user prompt or default block.
										//webviewWindow->CallDevToolsProtocolMethod(
										//	L"Browser.grantPermissions",
										//	jsonArgs.toWideCharPointer(),
										//	nullptr // We don't need the result
										//);

										// Execute the connector function with the valid ID
										String trigger = "window.connectToJuce('" + streamId + "');";
										sender->ExecuteScript(trigger.toWideCharPointer(), nullptr);
										return S_OK;
									}).Get(), &token);



							// --- 8. CREATE TEXTURE STREAM (NOW SAFE) ---
							// We create it now that the GPU process is fully linked to the controller.
							HRESULT hrStream = experimentalEnv->CreateTextureStream(
								streamIdW.c_str(),
								d3dDevice.get(),
								&textureStream
							);

							if (FAILED(hrStream)) {
								LOGERROR("Failed to CreateTextureStream: " + getErrorMessage(hrStream));
							}
							else {
								LOG("TextureStream Created Successfully!");
							}


							// Initialize listeners
							initTextureStream();

							loadURLOrFile();

							// 9. Navigate
							return S_OK;
						}).Get());

				return hrController;
			}).Get()
				);

	if (FAILED(hr)) {
		LOGERROR("Immediate Environment Creation Failed: " + getErrorMessage(hr));
	}
}
void WebViewMedia::initTextureStream()
{
	if (!textureStream) return;

	// Listen for new textures
	HRESULT hr = textureStream->add_WebTextureReceived(
		Callback<ICoreWebView2ExperimentalTextureStreamWebTextureReceivedEventHandler>(
			[this](ICoreWebView2ExperimentalTextureStream* sender, ICoreWebView2ExperimentalTextureStreamWebTextureReceivedEventArgs* args) -> HRESULT
			{

				// FIX 1: Change type to ICoreWebView2ExperimentalWebTexture (Note the "Web" in the name)
				wil::com_ptr<ICoreWebView2ExperimentalWebTexture> webTexture;

				// FIX 2: Call get_WebTexture instead of get_Texture
				args->get_WebTexture(&webTexture);

				if (webTexture)
				{
					HANDLE sharedHandle = nullptr;
					webTexture->get_Handle(&sharedHandle);

					if (sharedHandle)
					{
						wil::com_ptr<ID3D11Texture2D> openedTexture;
						HRESULT hrOpen = E_FAIL;

						// --- FIX START ---
						// 1. Query for ID3D11Device1 (Required for NT Handles)
						wil::com_ptr<ID3D11Device1> d3dDevice1;
						if (SUCCEEDED(d3dDevice->QueryInterface(IID_PPV_ARGS(&d3dDevice1))))
						{
							// 2. Use OpenSharedResource1
							// WebView2 provides NT Handles, which require this method.
							hrOpen = d3dDevice1->OpenSharedResource1(sharedHandle, IID_PPV_ARGS(&openedTexture));
						}
						else
						{
							// Fallback (Unlikely to work for TextureStream, but safe to keep)
							hrOpen = d3dDevice->OpenSharedResource(sharedHandle, IID_PPV_ARGS(&openedTexture));
						}
						// --- FIX END ---

						if (SUCCEEDED(hrOpen))
						{
							// ... (Your existing success logic) ...
							GenericScopedLock lock(textureLock);
							currentD3DTexture = openedTexture;
							newFrameAvailable = true;
							shouldRedraw = true;
							//LOG("New frame available here");
						}
						else
						{
							LOGERROR("OpenSharedResource Failed: " + getErrorMessage(hrOpen));
						}
					}
				}
				return S_OK;
			}).Get(), nullptr);

	if (FAILED(hr))
	{
		LOGERROR("Failed to add WebTextureReceived listener: " + getErrorMessage(hr));
		return;
	}

	isWebViewReady = true;
}

// Change return type to bool to indicate success
bool WebViewMedia::initD3D11()
{
	// Feature levels we support
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

	// BGRA Support is REQUIRED for WebView2 and Direct2D interop
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	//useful for debugging D3D issues, but remove for release if not needed
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL returnedFeatureLevel;

	// Explicitly nullify before creation just in case
	d3dDevice.reset();

	HRESULT hr = D3D11CreateDevice(
		nullptr,                    // Use default adapter (See note below on GPU Mismatch)
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&d3dDevice,                 // wil::com_ptr address-of operator handles this safely
		&returnedFeatureLevel,
		nullptr                     // We don't need the context immediately here
	);

	if (FAILED(hr))
	{
		LOGERROR("CRITICAL: D3D11CreateDevice Failed: " + getErrorMessage(hr));
		return false;
	}

	if (!d3dDevice)
	{
		LOGERROR("CRITICAL: D3D11 Device is NULL after successful creation?");
		return false;
	}

	LOG("D3D11 Device Created. Feature Level: " + String::toHexString((int)returnedFeatureLevel));

	// --- NEW LOGGING CODE START ---
	wil::com_ptr<IDXGIDevice> dxgiDevice;
	d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

	wil::com_ptr<IDXGIAdapter> adapter;
	dxgiDevice->GetAdapter(&adapter);

	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);

	String gpuName = String(desc.Description);
	int vendorId = desc.VendorId;

	// LUID is the most reliable way to compare devices programmatically
	String luidStr = String::toHexString((int)desc.AdapterLuid.HighPart) + "-" +
		String::toHexString((int)desc.AdapterLuid.LowPart);

	LOG("D3D11 Created on GPU: " + gpuName);
	LOG("GPU Vendor ID: " + String::toHexString(vendorId));
	LOG("Adapter LUID: " + luidStr);
	// --- NEW LOGGING CODE END ---

	// IMMEDIATELY Enable Multithread Protection
		// This is often required before passing the device to WebView2
	wil::com_ptr<ID3D11Multithread> multiThread;
	if (SUCCEEDED(d3dDevice->QueryInterface(IID_PPV_ARGS(&multiThread))))
	{
		multiThread->SetMultithreadProtected(TRUE);
		LOG("D3D11 Multithread Protection Enabled.");
	}
	else
	{
		LOGERROR("WARNING: ID3D11Multithread interface not found.");
	}

	return true;
}

// ==============================================================================
// OPENGL RENDERING & INTEROP
// ==============================================================================

void WebViewMedia::setupGLExtensions()
{
	// Load NV_DX_interop functions if not loaded
	if (!wglDXOpenDeviceNV)
	{
		wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)get_gl_proc_address("wglDXOpenDeviceNV");
		wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)get_gl_proc_address("wglDXRegisterObjectNV");
		wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)get_gl_proc_address("wglDXUnregisterObjectNV");
		wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)get_gl_proc_address("wglDXLockObjectsNV");
		wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)get_gl_proc_address("wglDXUnlockObjectsNV");
		wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)get_gl_proc_address("wglDXCloseDeviceNV");
	}
}

void WebViewMedia::initGLInternal()
{
	setupGLExtensions();

	// Create a local OpenGL Texture ID that will wrap the D3D texture
	glGenTextures(1, &glTextureID);
	glBindTexture(GL_TEXTURE_2D, glTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Register the D3D device with OpenGL
	if (d3dDevice && wglDXOpenDeviceNV)
	{
		glDxDeviceHandle = wglDXOpenDeviceNV(d3dDevice.get());
	}
}

void WebViewMedia::renderGLInternal()
{
	// 0. LAZY INIT (Handles the race condition)
	if (!glDxDeviceHandle && d3dDevice && wglDXOpenDeviceNV)
	{
		glDxDeviceHandle = wglDXOpenDeviceNV(d3dDevice.get());
		if (glDxDeviceHandle) LOG("GL-D3D Interop Connected");
	}

	// 1. UPDATE TEXTURE (Runs only when a new web frame arrives)
	if (newFrameAvailable)
	{
		GenericScopedLock lock(textureLock);

		if (currentD3DTexture && d3dDevice && glDxDeviceHandle)
		{
			D3D11_TEXTURE2D_DESC srcDesc;
			currentD3DTexture->GetDesc(&srcDesc);

			// Check if we need to Initialize or Resize the Intermediate Texture
			// This block ONLY runs once (or on resize)
			D3D11_TEXTURE2D_DESC currentDesc = {};
			if (intermediateTexture) intermediateTexture->GetDesc(&currentDesc);

			if (!intermediateTexture ||
				currentDesc.Width != srcDesc.Width ||
				currentDesc.Height != srcDesc.Height)
			{
				// Unregister old
				if (glIntermediateHandle) {
					wglDXUnregisterObjectNV(glDxDeviceHandle, glIntermediateHandle);
					glIntermediateHandle = nullptr;
				}

				// Create new Intermediate
				D3D11_TEXTURE2D_DESC copyDesc = srcDesc;
				copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
				copyDesc.MiscFlags = 0;
				copyDesc.CPUAccessFlags = 0;
				copyDesc.Usage = D3D11_USAGE_DEFAULT;
				copyDesc.MipLevels = 1; // <--- CRITICAL FORCE FIX
				copyDesc.ArraySize = 1; // <--- CRITICAL FORCE FIX

				if (SUCCEEDED(d3dDevice->CreateTexture2D(&copyDesc, nullptr, &intermediateTexture)))
				{
					//LOG("Intermediate Texture Created: " + String(srcDesc.Width) + "x" + String(srcDesc.Height));

					// Resize the GL ID to match
					glBindTexture(GL_TEXTURE_2D, glTextureID);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcDesc.Width, srcDesc.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
					glBindTexture(GL_TEXTURE_2D, 0);

					// Register NEW handle
					glIntermediateHandle = wglDXRegisterObjectNV(glDxDeviceHandle, intermediateTexture.get(), glTextureID, GL_TEXTURE_2D, WGL_ACCESS_READ_ONLY_NV);
				}
			}

			// COPY FRAME (Runs every frame)
			if (intermediateTexture)
			{
				wil::com_ptr<ID3D11DeviceContext> context;
				d3dDevice->GetImmediateContext(&context);

				// 1. ACQUIRE MUTEX (Mandatory for Shared Textures)
				wil::com_ptr<IDXGIKeyedMutex> mutex;
				// NOTE: Use IID_IDXGIKeyedMutex explicitly to be safe
				currentD3DTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&mutex);
				bool mutexAcquired = false;

				if (mutex)
				{
					// Wait up to 50ms to acquire the texture from the Browser Process
					HRESULT hrAcq = mutex->AcquireSync(0, 50);
					if (SUCCEEDED(hrAcq)) mutexAcquired = true;
					else LOGERROR("Failed to acquire Keyed Mutex for texture copy");
				}

				if (mutexAcquired || !mutex)
				{
					// 2. USE CopySubresourceRegion (Safer than CopyResource)
					// CopyResource crashes if MipLevels don't match exactly.
					// CopySubresourceRegion copies just the main image (Mip 0), which is safer.
					context->CopySubresourceRegion(
						intermediateTexture.get(), 0, 0, 0, 0, // Dest: Mip 0, (0,0,0)
						currentD3DTexture.get(), 0,            // Src:  Mip 0
						nullptr                                // Box:  nullptr = Whole texture
					);

					// 3. FLUSH (Ensure the command is submitted before we release)
					context->Flush();
				}

				// 4. RELEASE MUTEX
				if (mutexAcquired)
				{
					mutex->ReleaseSync(0);
				}
			}
		}
		newFrameAvailable = false;
	}

	// 2. RENDER (Must run every frame to keep the UI alive)
	if (glIntermediateHandle && wglDXLockObjectsNV)
	{
		// LOCK
		if (wglDXLockObjectsNV(glDxDeviceHandle, 1, &glIntermediateHandle))
		{
			// DRAW
			// Note: Media.cpp has already bound the Framebuffer, so we just draw.
			// Explicitly enable texture/blend states just in case
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindTexture(GL_TEXTURE_2D, glTextureID);

			// Set filtering to ensure it doesn't look like a white box
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Draw Quad
			Init2DViewport(width->intValue(), height->intValue());
			glClearColor(0, 0, 0, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			glColor4f(1, 1, 1, 1);
			Draw2DTexRectFlipped(0, 0, width->intValue(), height->intValue());

			glBindTexture(GL_TEXTURE_2D, 0);

			// UNLOCK (CRITICAL FIX: YOU MUST UNLOCK OR IT DIES NEXT FRAME)
			wglDXUnlockObjectsNV(glDxDeviceHandle, 1, &glIntermediateHandle);
		}
	}
}

void WebViewMedia::closeGLInternal()
{
	// Cleanup GL resources
	if (glDxSharedTextureHandle && wglDXUnregisterObjectNV)
	{
		wglDXUnregisterObjectNV(glDxDeviceHandle, glDxSharedTextureHandle);
		glDxSharedTextureHandle = nullptr;
	}

	if (glDxDeviceHandle && wglDXCloseDeviceNV)
	{
		wglDXCloseDeviceNV(glDxDeviceHandle);
		glDxDeviceHandle = nullptr;
	}

	if (glTextureID != 0)
	{
		glDeleteTextures(1, &glTextureID);
		glTextureID = 0;
	}
}

void WebViewMedia::loadURLOrFile()
{
	if (!isWebViewReady) return;

	bool isFile = source->getValueDataAsEnum<SourceType>() == Source_File;

	String origin;
	String urlToNavigate;

	if (isFile)
	{
		wil::com_ptr<ICoreWebView2_3> webview3;
		if (SUCCEEDED(webviewWindow->QueryInterface(IID_PPV_ARGS(&webview3))) && webview3)
		{
			File htmlFile = filePath->getFile();
			File folder = htmlFile.getParentDirectory();

			LOG("Setting to " << folder.getFullPathName());

			HRESULT hrMap = webview3->SetVirtualHostNameToFolderMapping(
				L"app.local",														// virtual host name
				(folder.getFullPathName()).toWideCharPointer(),                       // local folder
				COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);

			if (SUCCEEDED(hrMap))
			{
				origin = "https://app.local";
				LOG("Virtual host mapping set: " << origin << " -> " << folder.getFullPathName());

				// Build the URL we will actually navigate to
				urlToNavigate = origin + "/" + htmlFile.getFileName(); // Use virtual host
			}
			else
			{
				LOGERROR("SetVirtualHostNameToFolderMapping failed: " + getErrorMessage(hrMap) << " : " << folder.getFullPathName());
			}
		}
		else
		{
			LOGERROR("ICoreWebView2_3 not available; cannot set virtual host mapping.");
		}
	}
	else
	{
		// Very simple origin extraction: "<scheme>://<host>[:port]"
		urlToNavigate = urlParam->stringValue();
		URL url(urlParam->stringValue());
		int port = url.getPort();
		origin = url.getScheme() + "://" + url.getDomain();
		if (port != 80 && port != 0) origin += ":" + String(port);

		else
			origin = url.getScheme() + "://" + url.getDomain();
	}

	if (origin.isNotEmpty()) {
		std::wstring originW = origin.toWideCharPointer();
		HRESULT hrOrigin = textureStream->AddAllowedOrigin(originW.c_str(), TRUE);

		if (FAILED(hrOrigin)) {
			LOGERROR("AddAllowedOrigin failed for origin " + origin
				+ " : " + getErrorMessage(hrOrigin));
		}
		else {
			LOG("TextureStream allowed origin: " + origin);
		}
	}


	LOG("Navigating to : " + urlToNavigate);
	webviewWindow->Navigate(urlToNavigate.toWideCharPointer());
}

// ==============================================================================
// PARAMETERS & HELPERS
// ==============================================================================

void WebViewMedia::onContainerParameterChangedInternal(Parameter* p)
{
	Media::onContainerParameterChangedInternal(p);

	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<SourceType>() == Source_File;
		filePath->setEnabled(isFile);
		urlParam->setEnabled(!isFile);
	}

	if (p == source || p == filePath || p == urlParam)
	{
		loadURLOrFile();
	}
	else if (p == width || p == height)
	{
		if (controller)
		{
			RECT bounds = { 0, 0, width->intValue(), height->intValue() };
			controller->put_Bounds(bounds);
		}
	}
}

void WebViewMedia::onContainerTriggerTriggered(Trigger* t)
{
	if (t == reloadTrigger && webviewWindow)
	{
		webviewWindow->Reload();
	}
	else if (t == devToolsTrigger && webviewWindow)
	{
		webviewWindow->OpenDevToolsWindow();
	}
}

Point<int> WebViewMedia::getDefaultMediaSize()
{
	return Point<int>(width->intValue(), height->intValue());
}

// ==============================================================================
// INPUT HANDLING
// ==============================================================================

Point<int> WebViewMedia::getMediaMousePosition(const MouseEvent& e, Rectangle<int> canvasRect)
{
	// Map screen mouse to WebView coordinates
	int tx = (e.getPosition().x - canvasRect.getX()) * width->intValue() / canvasRect.getWidth();
	int ty = (e.getPosition().y - canvasRect.getY()) * height->intValue() / canvasRect.getHeight();
	return Point<int>(tx, ty);
}

void WebViewMedia::sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect)
{
	if (!controller) return;
	Point<int> p = getMediaMousePosition(e, canvasRect);

	COREWEBVIEW2_MOUSE_EVENT_KIND kind = e.mods.isRightButtonDown() ?
		COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN : COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN;

	compositionController->SendMouseInput(kind, COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE, 0, { p.x, p.y });
}

void WebViewMedia::sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect)
{
	if (!controller) return;
	Point<int> p = getMediaMousePosition(e, canvasRect);

	COREWEBVIEW2_MOUSE_EVENT_KIND kind = e.mods.isRightButtonDown() ?
		COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP : COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP;

	compositionController->SendMouseInput(kind, COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE, 0, { p.x, p.y });
}

void WebViewMedia::sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect)
{
	if (!controller) return;
	Point<int> p = getMediaMousePosition(e, canvasRect);

	compositionController->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE, COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE, 0, { p.x, p.y });
}

void WebViewMedia::sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect)
{
	sendMouseMove(e, canvasRect); // WebView2 treats drag as move with button down
}

void WebViewMedia::sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
	// Implementation omitted for brevity (similar to SendMouseInput but using mouseData for wheel delta)
}

void WebViewMedia::sendKeyPressed(const KeyPress& key)
{
	// WebView2 Input for Keyboard is complex (needs TranslateAccelerator usually).
	// For offscreen, standard SendInput approach isn't sufficient without HWND focus.
	// You might need to inject JS keyboard events if SendInput fails.
}