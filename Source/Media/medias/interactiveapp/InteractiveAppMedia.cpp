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
#include "InteractiveAppMedia.h"
#endif

InteractiveAppMedia::InteractiveAppMedia(var params) :
	BaseSharedTextureMedia(getTypeString(), params),
	checkingProcess(false)
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
		if(curSharingName.isNotEmpty()) availableTextures->setValueWithKey(curSharingName);
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

	File wDir = File::getCurrentWorkingDirectory();
	f.getParentDirectory().setAsCurrentWorkingDirectory();
	bool result = f.startAsProcess(launchArguments->stringValue());
	wDir.setAsCurrentWorkingDirectory();

	if(!result) LOGERROR("Could not launch application " << f.getFullPathName() << " with arguments : " << launchArguments->stringValue());
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
