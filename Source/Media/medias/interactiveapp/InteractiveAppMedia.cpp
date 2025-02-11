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
	shouldMinimize(false)
{
	appParam = addFileParameter("App", "App", "");
	launchArguments = addStringParameter("Launch Arguments", "Launch Arguments", "");
	launchArguments->multiline = true;

	availableTextures = addEnumParameter("Available Textures", "Available Textures");
	availableTextures->saveValueOnly = false;

	appRunning = addBoolParameter("App Running", "App Running", false);

	autoStartOnPreUse = addBoolParameter("Auto Start On Pre Use", "Auto Start On Pre Use", false);
	autoStartOnUse = addBoolParameter("Auto Start On Use", "Auto Start On Use", false);
	autoStopOnUse = addBoolParameter("Auto Stop On Use", "Auto Stop On Use", false);
	launchMinimized = addBoolParameter("Launch Minimized", "Launch Minimized", false);

	hardKill = addBoolParameter("Hard Kill", "Hard Kill", false);

	sharingName->hideInEditor = true;

	startTimerHz(2);
}

InteractiveAppMedia::~InteractiveAppMedia()
{
	stopTimer();
}

void InteractiveAppMedia::onContainerParameterChangedInternal(Parameter* p)
{
	BaseSharedTextureMedia::onContainerParameterChangedInternal(p);

	if (p == appRunning)
	{
		if (!checkingProcess)
		{
			if (appRunning->boolValue()) launchProcess();
			else killProcess();
		}
	}
	else if (p == availableTextures)
	{
		sharingName->setValue(availableTextures->stringValue());
	}
}

void InteractiveAppMedia::checkAppRunning()
{
	checkingProcess = true;
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

	if (isRunning && !isThreadRunning())
	{
#if JUCE_WINDOWS
		startThread();
#endif
	}

	appRunning->setValue(isRunning);
	checkingProcess = false;
}

void InteractiveAppMedia::updateTextureList()
{
	if (!appRunning->boolValue()) return; //only update when app is running

	StringArray senders = SharedTextureManager::getInstance()->getAvailableSenders();
	StringArray keys = availableTextures->getAllKeys();


	StringArray goodSenders;
	for (auto& s : senders)
	{
		if (s.contains(niceName)) goodSenders.add(s);
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
		if (!appRunning->boolValue()) launchProcess();
	}
	else if (shouldKill)
	{
		if (appRunning->boolValue()) killProcess();
	}
}

void InteractiveAppMedia::launchProcess()
{
	if (checkingProcess || isClearing) return;

	File f = appParam->getFile();
	if (!f.exists())
	{
		NLOGWARNING(niceName, "File does not exist : " + f.getFullPathName());
	}

	String args = launchArguments->stringValue();


	File wDir = File::getCurrentWorkingDirectory();
	f.getParentDirectory().setAsCurrentWorkingDirectory();
	bool result = f.startAsProcess(args);
	wDir.setAsCurrentWorkingDirectory();

	if (launchMinimized->boolValue()) shouldMinimize = true;

	if (!result) LOGERROR("Could not launch application " << f.getFullPathName() << " with arguments : " << launchArguments->stringValue());
}

void InteractiveAppMedia::killProcess()
{
	File f = appParam->getFile();
	if (checkingProcess || !f.existsAsFile()) return;

	String processName = f.getFileName();
	bool hardKillMode = hardKill->boolValue();

#if JUCE_WINDOWS
	int result = WinExec(String("taskkill " + String(hardKillMode ? "/f " : "") + "/im \"" + processName + "\"").getCharPointer(), SW_HIDE);
	if (result < 31) LOGWARNING("Problem killing app " + processName);
#else
	int result = system(String("killall " + String(hardKillMode ? "-9" : "-2") + " \"" + processName + "\"").getCharPointer());
	if (result != 0) LOGWARNING("Problem killing app " + processName);
#endif
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
	while (!threadShouldExit() && shouldMinimize)
	{
		EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(this));
		wait(1000);
	}
#endif
}