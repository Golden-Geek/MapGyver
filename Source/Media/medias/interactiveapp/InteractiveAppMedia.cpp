/*
  ==============================================================================

	InteractiveAppMedia.cpp
	Created: 11 Feb 2025 3:01:59pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

#if JUCE_WINDOWS
#include <TlHelp32.h>
#endif

#include "InteractiveAppMedia.h"

InteractiveAppMedia::InteractiveAppMedia(var params) :
	BaseSharedTextureMedia(getTypeString(), params),
	Thread("Interactive App"),
	checkingProcess(false),
	shouldMinimize(false),
	shouldSynchronize(false),
	isUpdatingStructure(false),
	hasListenExtension(false)
{
	appParam = addFileParameter("App", "App", "");
	launchArguments = addStringParameter("Launch Arguments", "Launch Arguments", "");
	launchArguments->multiline = true;

	availableTextures = addEnumParameter("Available Textures", "Available Textures");
	availableTextures->saveValueOnly = false;

	appState = addEnumParameter("App State", "App State");
	appState->addOption("Closed", CLOSED)->addOption("Launching", LAUNCHING)->addOption("Running", RUNNING)->addOption("Closing", CLOSING);
	appState->setControllableFeedbackOnly(true);

	autoStartOnPreUse = addBoolParameter("Auto Start On Pre Use", "Auto Start On Pre Use", false);
	autoStartOnUse = addBoolParameter("Auto Start On Use", "Auto Start On Use", false);
	autoStopOnUse = addBoolParameter("Auto Stop On Use", "Auto Stop On Use", false);
	stopOnClear = addBoolParameter("Stop On Clear", "Stop when this media is removed or the app closes", false);
	launchMinimized = addBoolParameter("Launch Minimized", "Launch Minimized", false);

	hardKill = addBoolParameter("Hard Kill", "Hard Kill", false);

	oscQueryPort = addIntParameter("OSC Query Port", "OSC Query Port", 9010);
	keepValuesOnSync = addBoolParameter("Keep Values On Sync", "Keep Values On Sync", false);

	serverName = addStringParameter("Server Name", "Server Name", "");
	serverName->setControllableFeedbackOnly(true);
	isConnected = addBoolParameter("Is Connected", "Is Connected", false);
	isConnected->setControllableFeedbackOnly(true);

	doNotPreview->setDefaultValue(true);

	sharingName->hideInEditor = true;

	oscSender.connect("0.0.0.0", 0);

	startTimerHz(2);
}

InteractiveAppMedia::~InteractiveAppMedia()
{
	stopTimer();
}

void InteractiveAppMedia::clearItem()
{
	stopTimer();
	stopThread(1000);
	if (stopOnClear->boolValue()) killProcess();

	if (wsClient != nullptr) wsClient->stop();

	Media::clearItem();
}

void InteractiveAppMedia::onContainerParameterChangedInternal(Parameter* p)
{
	BaseSharedTextureMedia::onContainerParameterChangedInternal(p);

	if (p == appParam)
	{
		mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
	}
	else if (p == appState)
	{
		//if (!checkingProcess)
		//{
		//	if (appState->getValueDataAsEnum<AppState>() == AppState::RUNNING) launchProcess();
		//	else killProcess();
		//}
	}
	else if (p == availableTextures)
	{
		sharingName->setValue(availableTextures->stringValue());
	}
}

void InteractiveAppMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (cc == &mediaParams)
	{
		if (OSCQueryHelpers::OSCQueryValueContainer* gcc = c->getParentAs<OSCQueryHelpers::OSCQueryValueContainer>())
		{
			if (c == gcc->enableListen)
			{
				updateListenToContainer(gcc);
			}
			else if (c == gcc->syncContent)
			{
				shouldSynchronize = true;
				if (!isThreadRunning()) startThread();
			}
			else
			{
				sendOSCForControllable(c);
			}
		}
		else
		{
			sendOSCForControllable(c);
		}
	}
}

void InteractiveAppMedia::checkAppRunning()
{

	StringArray processes;

#if JUCE_WINDOWS
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		LOGERROR("OS List process : Invalid handle value");
	}
	else {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) {
			processes.add(pe32.szExeFile);
			while (Process32Next(hProcessSnap, &pe32)) processes.add(pe32.szExeFile);
		}
	}
	// clean the snapshot object
	CloseHandle(hProcessSnap);
#elif JUCE_MAC
#elif JUCE_LINUX
#endif

	bool isRunning = processes.contains(appParam->getFile().getFileName());

	if (isRunning && (shouldMinimize || shouldSynchronize) && !isThreadRunning())
	{
#if JUCE_WINDOWS
		startThread();
#endif
	}


	checkingProcess = true;
	appState->setValueWithData(isRunning ? AppState::RUNNING : AppState::CLOSED);
	checkingProcess = false;
}

bool InteractiveAppMedia::isAppRunning()
{
	return appState->getValueDataAsEnum<AppState>() == AppState::RUNNING;
}

void InteractiveAppMedia::updateTextureList()
{
	if (!isAppRunning()) return; //only update when app is running

	StringArray senders = SharedTextureManager::getInstance()->getAvailableSenders();
	StringArray keys = availableTextures->getAllKeys();


	StringArray goodSenders;
	for (auto& s : senders)
	{
		if (s.contains(serverName->stringValue())) goodSenders.add(s);
	}

	if (goodSenders.isEmpty()) return; //if something went bad, don't destroy the detected list

	bool listHasChanged = false;
	for (auto& s : goodSenders)
	{
		if (!keys.contains(s))
		{
			availableTextures->addOption(s, s);
			listHasChanged = true;
		}
	}

	for (auto& k : keys)
	{
		if (!goodSenders.contains(k))
		{
			availableTextures->removeOption(k);
			listHasChanged = true;
		}
	}

	if (listHasChanged)
	{
		String curSharingName = sharingName->stringValue();
		Array<EnumParameter::EnumValue> options;
		for (auto& s : goodSenders) options.add(EnumParameter::EnumValue(s, s));
		availableTextures->setOptions(options);
		if (curSharingName.isNotEmpty()) availableTextures->setValueWithKey(curSharingName);
	}
}

void InteractiveAppMedia::updateBeingUsed()
{
	BaseSharedTextureMedia::updateBeingUsed();

	bool isPreUsed = isBeingUsed->boolValue() && usedTargets.isEmpty();

	bool shouldLaunch = false;
	bool shouldKill = false;

	if (isBeingUsed->boolValue() && autoStartOnUse->boolValue()) shouldLaunch = true;
	if (isPreUsed && autoStartOnPreUse->boolValue()) shouldLaunch = true;
	if ((!isBeingUsed->boolValue() || isPreUsed) && autoStopOnUse->boolValue()) shouldKill = true;

	if (shouldLaunch)
	{
		launchProcess();
	}
	else if (shouldKill)
	{
		killProcess();
	}
}

void InteractiveAppMedia::syncOSCQuery()
{
	requestHostInfo();
}



void InteractiveAppMedia::requestHostInfo()
{
	isConnected->setValue(false);
	hasListenExtension = false;

	URL url("http://127.0.0.1:" + String(oscQueryPort->intValue()) + "?HOST_INFO");
	StringPairArray responseHeaders;
	int statusCode = 0;

	std::unique_ptr<InputStream> stream(url.createInputStream(
		URL::InputStreamOptions(URL::ParameterHandling::inAddress)
		.withConnectionTimeoutMs(2000)
		.withResponseHeaders(&responseHeaders)
		.withStatusCode(&statusCode)
	));

	bool success = false;

#if JUCE_WINDOWS
	if (statusCode != 200)
	{
		stream.reset();
	}
#endif

	if (stream != nullptr)
	{
		String content = stream->readEntireStreamAsString();
		//if (logIncomingData->boolValue()) NLOG(niceName, "Request status code : " << statusCode << ", content :\n" << content);

		var data = JSON::parse(content);
		if (data.isObject())
		{
			//if (logIncomingData->boolValue()) NLOG(niceName, "Received HOST_INFO :\n" << JSON::toString(data));

			success = true;

			String curServerName = serverName->stringValue();
			String newServerName = data.getProperty("NAME", "");
			if (curServerName.isNotEmpty() && newServerName != curServerName)
			{
				NLOG(niceName, "Server name has changed, if you wish to not sync, please enable \"Only sync from same name\"");
			}

			serverName->setValue(newServerName);


			//String oscIP = data.getProperty("OSC_IP", remoteHost->stringValue());
			/*int oscPort = data.getProperty("OSC_PORT", remotePort->intValue());
			remoteOSCPort->setEnabled(oscPort != remotePort->intValue());
			remoteOSCPort->setValue(oscPort);
			if (oscPort != remotePort->intValue()) NLOG(niceName, "OSC_PORT is different from OSCQuery port, setting custom OSC port to " << oscPort);

			int wsPort = data.getProperty("WS_PORT", remotePort->intValue());
			remoteWSPort->setEnabled(wsPort != remotePort->intValue());
			remoteWSPort->setValue(wsPort);
			if (wsPort != remotePort->intValue()) NLOG(niceName, "WS_PORT is different from OSCQuery port, setting custom Websocket port to " << wsPort);*/


			hasListenExtension = data.getProperty("EXTENSIONS", var()).getProperty("LISTEN", false);
			requestStructure();

			if (hasListenExtension) NLOG(niceName, "Server has LISTEN extension, setting up websocket");
			setupWSAndOSC();

			shouldSynchronize = false;
		}

	}

	if (!success)
	{
		setWarningMessage("Can't connect to OSCQuery server", "sync");

		if (getWarningMessage("sync").isEmpty()) NLOGERROR(niceName, "Error with host info request, status code : " << statusCode << ", url : " << url.toString(true));
	}
	else
	{
		clearWarning("sync");
	}
}

void InteractiveAppMedia::requestStructure()
{
	URL url("http://127.0.0.1:" + String(oscQueryPort->intValue()));
	StringPairArray responseHeaders;
	int statusCode = 0;

	std::unique_ptr<InputStream> stream(url.createInputStream(
		URL::InputStreamOptions(URL::ParameterHandling::inAddress)
		.withConnectionTimeoutMs(15000)
		.withResponseHeaders(&responseHeaders)
		.withStatusCode(&statusCode)
	));

#if JUCE_WINDOWS
	if (statusCode != 200)
	{
		NLOGWARNING(niceName, "Failed to request Structure, status code = " + String(statusCode));
		return;
	}
#endif


	if (stream != nullptr)
	{
		String content = stream->readEntireStreamAsString();
		//if (logIncomingData->boolValue()) NLOG(niceName, "Request status code : " << statusCode << ", content :\n" << content);

		var data = JSON::parse(content);
		if (data.isObject())
		{
			//if (logIncomingData->boolValue()) NLOG(niceName, "Received structure :\n" << JSON::toString(data));

			updateTreeFromData(data);
		}
	}
	else
	{
		//if (logIncomingData->boolValue()) NLOGWARNING(niceName, "Error with request, status code : " << statusCode << ", url : " << url.toString(true));
	}
}

void InteractiveAppMedia::updateTreeFromData(var data)
{
	if (data.isVoid()) return;

	isUpdatingStructure = true;

	Array<String> enableListenContainers;
	Array<String> expandedContainers;
	Array<WeakReference<ControllableContainer>> containers = mediaParams.getAllContainers(true);

	if (!keepValuesOnSync->boolValue())
	{
		for (auto& cc : containers)
		{
			if (OSCQueryHelpers::OSCQueryValueContainer* gcc = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(cc.get()))
			{
				if (gcc->enableListen->boolValue())
				{
					enableListenContainers.add(gcc->getControlAddress(&mediaParams));
					gcc->enableListen->setValue(false);
					if (!gcc->editorIsCollapsed) expandedContainers.add(gcc->getControlAddress(&mediaParams));
				}
			}
		}
	}


	var vData(new DynamicObject());
	if (keepValuesOnSync->boolValue())
	{
		Array<WeakReference<Parameter>> params = mediaParams.getAllParameters(true);
		for (auto& p : params)
		{
			vData.getDynamicObject()->setProperty(p->getControlAddress(&mediaParams), p->value);
		}

		for (auto& cc : containers)
		{
			if (OSCQueryHelpers::OSCQueryValueContainer* gcc = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(cc.get()))
			{
				if (gcc->enableListen->boolValue()) gcc->enableListen->setValue(true, false, true); //force relistening
			}
		}
	}

	//mediaParams.clear();

	OSCQueryHelpers::updateContainerFromData(&mediaParams, data, false);

	isUpdatingStructure = false;

	if (keepValuesOnSync->boolValue())
	{
		NamedValueSet nvs = vData.getDynamicObject()->getProperties();
		for (auto& nv : nvs)
		{
			if (Parameter* p = dynamic_cast<Parameter*>(mediaParams.getControllableForAddress(nv.name.toString())))
			{
				p->setValue(nv.value);
			}
		}
	}
	else
	{
		for (auto& addr : enableListenContainers)
		{
			if (OSCQueryHelpers::OSCQueryValueContainer* gcc = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(mediaParams.getControllableContainerForAddress(addr)))
			{
				gcc->enableListen->setValue(true, false, true);
			}
		}

		for (auto& addr : expandedContainers)
		{
			if (ControllableContainer* cc = mediaParams.getControllableContainerForAddress(addr))
			{
				cc->editorIsCollapsed = false;
				cc->queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerCollapsedChanged, cc)); //should move to a setCollapsed from ControllableContainer.cpp
			}
		}
	}

	treeData = data;
}

void InteractiveAppMedia::setupWSAndOSC()
{
	isConnected->setValue(false);
	if (wsClient != nullptr) wsClient->stop();
	wsClient.reset();
	if (isCurrentlyLoadingData || !hasListenExtension) return;

	if (!enabled->boolValue()) return;
	wsClient.reset(new SimpleWebSocketClient());
	wsClient->addWebSocketListener(this);

	String host = "127.0.0.1";
	String wsPort = oscQueryPort->stringValue();
	String url = host + ":" + wsPort + "/";
	wsClient->start(url);
}

void InteractiveAppMedia::sendOSC(const OSCMessage& m)
{
	if (!enabled->boolValue()) return;

	//if (logOutgoingData->boolValue())
	//{
	//	NLOG(niceName, "Send OSC : " << m.getAddressPattern().toString());
	//	for (auto& a : m) LOG(OSCHelpers::getStringArg(a));
	//}

	//outActivityTrigger->trigger();

	String host = "127.0.0.1";
	int port = oscQueryPort->intValue();
	oscSender.sendToIPAddress(host, port, m);
}

void InteractiveAppMedia::sendOSCForControllable(Controllable* c)
{
	if (!enabled->boolValue()) return;
	if (isUpdatingStructure) return;
	if (isCurrentlyLoadingData) return;
	if (noFeedbackList.contains(c)) return;


	String s = c->getControlAddress(&mediaParams);
	try
	{
		OSCMessage m(s);
		if (c->type != Controllable::TRIGGER)
		{
			Parameter* p = (Parameter*)c;
			var v = p->getValue().clone();

			if (v.isArray() && p->type != Controllable::COLOR)
			{
				for (int i = 0; i < v.size(); ++i)
				{
					m.addArgument(OSCHelpers::varToArgument(v[i], OSCHelpers::BoolMode::TF));
				}
			}
			else
			{
				m.addArgument(OSCHelpers::varToArgument(v, OSCHelpers::BoolMode::TF));
			}
		}
		sendOSC(m);
	}
	catch (OSCFormatError& e)
	{
		NLOGERROR(niceName, "Can't send to address " << s << " : " << e.description);
	}
}


OSCArgument InteractiveAppMedia::varToArgument(const var& v)
{
	if (v.isBool()) return OSCArgument(((bool)v) ? 1 : 0);
	else if (v.isInt()) return OSCArgument((int)v);
	else if (v.isInt64()) return OSCArgument((int)v);
	else if (v.isDouble()) return OSCArgument((float)v);
	else if (v.isString()) return OSCArgument(v.toString());
	jassert(false);
	return OSCArgument("error");
}

void InteractiveAppMedia::updateAllListens()
{
	Array<WeakReference<ControllableContainer>> containers = mediaParams.getAllContainers(true);
	for (auto& cc : containers)
	{
		if (OSCQueryHelpers::OSCQueryValueContainer* gcc = dynamic_cast<OSCQueryHelpers::OSCQueryValueContainer*>(cc.get()))
		{
			updateListenToContainer(gcc, true);
		}
	}
}

void InteractiveAppMedia::updateListenToContainer(OSCQueryHelpers::OSCQueryValueContainer* gcc, bool onlySendIfListen)
{
	if (!enabled->boolValue() || !hasListenExtension || isCurrentlyLoadingData || isUpdatingStructure) return;
	if (wsClient == nullptr || !wsClient->isConnected)
	{
		NLOGWARNING(niceName, "Websocket not connected, can't LISTEN");
		return;
	}

	if (onlySendIfListen && !gcc->enableListen->boolValue()) return;

	String command = gcc->enableListen->boolValue() ? "LISTEN" : "IGNORE";
	Array<WeakReference<Controllable>> params = gcc->getAllControllables();

	var o(new DynamicObject());
	o.getDynamicObject()->setProperty("COMMAND", command);

	for (auto& p : params)
	{
		if (p == gcc->enableListen || p == gcc->syncContent) continue;
		String addr = p->getControlAddress(&mediaParams);
		o.getDynamicObject()->setProperty("DATA", addr);
		wsClient->send(JSON::toString(o, true));
	}
}

void InteractiveAppMedia::connectionOpened()
{
	NLOG(niceName, "Websocket connection is opened, let's get bi, baby !");
	isConnected->setValue(true);
	clearWarning("sync");
	updateAllListens();
}

void InteractiveAppMedia::connectionClosed(int status, const String& reason)
{
	NLOG(niceName, "Websocket connection is closed, bye bye!");
	isConnected->setValue(false);
}

void InteractiveAppMedia::connectionError(const String& errorMessage)
{
	NLOGERROR(niceName, "Connection error " << errorMessage);
	isConnected->setValue(false);
}

void InteractiveAppMedia::dataReceived(const MemoryBlock& data)
{
	if (!enabled->boolValue()) return;

	OSCPacketParser parser(data.getData(), (int)data.getSize());
	OSCMessage m = parser.readMessage();

	processOSCMessage(m);
}

void InteractiveAppMedia::processOSCMessage(const OSCMessage& m)
{
	/*if (logIncomingData->boolValue())
	{
		String s = m.getAddressPattern().toString();
		for (auto& a : m) s += "\n" + OSCHelpers::getStringArg(a);
		NLOG(niceName, "Feedback received : " << s);
	}*/

	//inActivityTrigger->trigger();

	if (Controllable* c = OSCHelpers::findControllable(&mediaParams, m))
	{
		noFeedbackList.add(c);
		OSCHelpers::handleControllableForOSCMessage(c, m);
		noFeedbackList.removeAllInstancesOf(c);
	}
}

void InteractiveAppMedia::messageReceived(const String& message)
{
	if (!enabled->boolValue()) return;

	//if (logIncomingData->boolValue())
	//{
	//	NLOG(niceName, "Websocket message received : " << message);
	//}

	//inActivityTrigger->trigger();
}

void InteractiveAppMedia::launchProcess()
{
	if (checkingProcess || isClearing) return;
	
	AppState state = appState->getValueDataAsEnum<AppState>();
	if (state == LAUNCHING || state == RUNNING) return;


	File f = appParam->getFile();
	if (!f.exists())
	{
		NLOGWARNING(niceName, "File does not exist : " + f.getFullPathName());
		appState->setValueWithData(CLOSED);
		return;
	}


	String args = launchArguments->stringValue();


	File wDir = File::getCurrentWorkingDirectory();
	f.getParentDirectory().setAsCurrentWorkingDirectory();
	bool result = f.startAsProcess(args);
	wDir.setAsCurrentWorkingDirectory();


	if (!result)
	{
		LOGERROR("Could not launch application " << f.getFullPathName() << " with arguments : " << launchArguments->stringValue());
		appState->setValueWithData(CLOSED);
		return;
	}

	appState->setValueWithData(LAUNCHING);

	shouldMinimize = result && launchMinimized->boolValue();
	shouldSynchronize = result;
}


void InteractiveAppMedia::killProcess()
{
	File f = appParam->getFile();
	if (checkingProcess || !f.existsAsFile()) return;

	AppState state = appState->getValueDataAsEnum<AppState>();
	if (state == CLOSING || state == CLOSED) return;

	appState->setValueWithData(CLOSING);

	if (wsClient != nullptr) wsClient->stop();

	String processName = f.getFileName();
	bool hardKillMode = hardKill->boolValue();

#if JUCE_WINDOWS
	int result = WinExec(String("taskkill " + String(hardKillMode ? "/f " : "") + "/im \"" + processName + "\"").getCharPointer(), SW_HIDE);
	if (result < 31) LOGWARNING("Problem killing app " + processName);
#else
	int result = system(String("killall " + String(hardKillMode ? "-9" : "-2") + " \"" + processName + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + processName);
#endif

	shouldMinimize = false;
	shouldSynchronize = false;
}

void InteractiveAppMedia::timerCallback()
{
	checkAppRunning();
	updateTextureList();
}

#if JUCE_WINDOWS
BOOL InteractiveAppMedia::enumWindowCallback(HWND hWnd, LPARAM lparam)
{


	InteractiveAppMedia* c = reinterpret_cast<InteractiveAppMedia*>(lparam);
	String pName = c->appParam->getFile().getFileName();

	if (!c->shouldMinimize) return FALSE;


	String hwndProcessName = "";
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hProcess)
	{
		TCHAR processName[MAX_PATH] = TEXT("<unknown>");
		DWORD processNameLength = MAX_PATH;
		//Get only file name process, like "App.exe" not the full path
		QueryFullProcessImageName(hProcess, 0, processName, &processNameLength);
		String fullPath = String(processName);
		hwndProcessName = fullPath.fromLastOccurrenceOf("\\", false, true);
		CloseHandle(hProcess);
	}

	if (pName == hwndProcessName)
	{
		//check if already minimized 
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &wp);
		if (wp.showCmd != SW_MINIMIZE) ShowWindow(hWnd, SW_MINIMIZE);
		c->shouldMinimize = false;
	}

	return TRUE;
}
#endif


void InteractiveAppMedia::run()
{
#if JUCE_WINDOWS
	while (!threadShouldExit() && (shouldMinimize || shouldSynchronize))
	{
		if (shouldMinimize)
		{
			EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(this));
		}

		wait(1000);

		if (shouldSynchronize)
		{
			syncOSCQuery();
		}
	}
#endif
}

String InteractiveAppMedia::getMediaContentName() const
{
	File f = appParam->getFile();
	return f.existsAsFile() ? f.getFileNameWithoutExtension() : Media::getMediaContentName();
}

var InteractiveAppMedia::getJSONData(bool includeNonOverriden)
{
	var data = BaseSharedTextureMedia::getJSONData(includeNonOverriden);
	data.getDynamicObject()->setProperty("treeData", treeData);
	return data;
}


void InteractiveAppMedia::loadJSONDataInternal(var data)
{
	BaseSharedTextureMedia::loadJSONDataInternal(data);
	var tData = data.getProperty("treeData", var());
	updateTreeFromData(tData);
}
