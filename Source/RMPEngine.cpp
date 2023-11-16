/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */
#include "RMPEngine.h"
#include "JuceHeader.h"

#include "Definitions/Screen/ScreenManager.h"
#include "Definitions/Media/MediaManager.h"
#include "NDIManager.h"

ControllableContainer* getAppSettings();

RMPEngine::RMPEngine() :
	Engine("RuleMaPool", ".coop")
	//defaultBehaviors("Test"),
	//ossiaFixture(nullptr)
{
	convertURL = "http://hazlab.fr/";
	
	//Communication
	// OSCRemoteControl::getInstance()->addRemoteControlListener(UserInputManager::getInstance());
	//init here
	Engine::mainEngine = this;
	RMPEngine::mainEngine = this;

	GlobalSettings::getInstance()->altScaleFactor->setDefaultValue(0.05);

	const char* argv[1] = { "-vvv" };
	VLCInstance = libvlc_new(1, argv);

	addChildControllableContainer(MediaManager::getInstance());
	addChildControllableContainer(ScreenManager::getInstance());

	// MIDIManager::getInstance(); //Trigger constructor, declare settings

	// getAppSettings()->addChildControllableContainer(&defaultBehaviors);

	NDIManager::getInstance();
}

RMPEngine::~RMPEngine()
{
	//Application-end cleanup, nothing should be recreated after this
	//delete singletons here

	isClearing = true;

#if JUCE_WINDOWS
	//WindowsHooker::deleteInstance();
#endif

	// ZeroconfManager::deleteInstance();
	// CommunityModuleManager::deleteInstance();
	// ModuleRouterManager::deleteInstance();

	// ChataigneSequenceManager::deleteInstance();
	// StateManager::deleteInstance();
	// ModuleManager::deleteInstance();

	// MIDIManager::deleteInstance();
	// DMXManager::deleteInstance();
	// SerialManager::deleteInstance();
	// WiimoteManager::deleteInstance();

	// InputSystemManager::deleteInstance();
	// StreamDeckManager::deleteInstance();

	// ChataigneAssetManager::deleteInstance();

	// CVGroupManager::deleteInstance();

	// Guider::deleteInstance();
	MediaManager::deleteInstance();
	ScreenManager::deleteInstance();
	NDIManager::deleteInstance();
	libvlc_release(VLCInstance); 
	VLCInstance = nullptr;
}


void RMPEngine::clearInternal()
{
	//clear
	// StateManager::getInstance()->clear();
	// ChataigneSequenceManager::getInstance()->clear();

	// ModuleRouterManager::getInstance()->clear();
	// ModuleManager::getInstance()->clear();
	ScreenManager::getInstance()->clear();
	MediaManager::getInstance()->clear();
}

var RMPEngine::getJSONData()
{
	var data = Engine::getJSONData();

	//var mData = ModuleManager::getInstance()->getJSONData();
	//if (!mData.isVoid() && mData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(ModuleManager::getInstance()->shortName, mData);

	var MediaData = MediaManager::getInstance()->getJSONData();
	if (!MediaData.isVoid() && MediaData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(MediaManager::getInstance()->shortName, MediaData);

	var ScreenData = ScreenManager::getInstance()->getJSONData();
	if (!ScreenData.isVoid() && ScreenData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(ScreenManager::getInstance()->shortName, ScreenData);


	return data;
}

void RMPEngine::loadJSONDataInternalEngine(var data, ProgressTask* loadingTask)
{
	//ProgressTask* moduleTask = loadingTask->addTask("Modules");
	ProgressTask* ScreenTask = loadingTask->addTask("Screens");
	ProgressTask* MediaTask = loadingTask->addTask("Medias");


	MediaTask->start();
	MediaManager::getInstance()->loadJSONData(data.getProperty(MediaManager::getInstance()->shortName, var()));
	MediaTask->setProgress(1);
	MediaTask->end();



	ScreenTask->start();
	ScreenManager::getInstance()->loadJSONData(data.getProperty(ScreenManager::getInstance()->shortName, var()));
	ScreenTask->setProgress(1);
	ScreenTask->end();


}

void RMPEngine::childStructureChanged(ControllableContainer* cc)
{
	Engine::childStructureChanged(cc);
}

void RMPEngine::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	if (isClearing || isLoadingFile) return;
}

void RMPEngine::handleAsyncUpdate()
{
	Engine::handleAsyncUpdate();
}

String RMPEngine::getMinimumRequiredFileVersion()
{
	return "1.0.0";
}

void RMPEngine::importSelection()
{
	const MessageManagerLock mmLock;
	FileChooser fc("Load some files", File::getCurrentWorkingDirectory(), "*.mochi");
	/*
	if (!fc.browseForMultipleFilesToOpen()) return;
	Array<File> f = fc.getResults();
	for (int i = 0; i < f.size(); i++) {
		if (f[i].hasFileExtension("mochi")) {
			importMochi(f[i]);
		}
	}
	*/
}

void RMPEngine::importMochi(var data)
{
	if (!data.isObject()) return;

	ScreenManager::getInstance()->addItemsFromData(data.getProperty(ScreenManager::getInstance()->shortName, var()));
	MediaManager::getInstance()->addItemsFromData(data.getProperty(MediaManager::getInstance()->shortName, var()));
}

void RMPEngine::exportSelection()
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty(ScreenManager::getInstance()->shortName, ScreenManager::getInstance()->getExportSelectionData());
	data.getDynamicObject()->setProperty(MediaManager::getInstance()->shortName, MediaManager::getInstance()->getExportSelectionData());

	String s = JSON::toString(data);
	
	/*
	FileChooser fc("Save a mochi", File::getCurrentWorkingDirectory(), "*.mochi");
	if (fc.browseForFileToSave(true))
	{
		File f = fc.getResult();
		f.replaceWithText(s);
	}
	*/
}

void RMPEngine::parameterValueChanged(Parameter* p) {
	Engine::parameterValueChanged(p);
	/* if (p == panelScale) {
		InputPanel::getInstance()->resized();
	}
	*/

}
