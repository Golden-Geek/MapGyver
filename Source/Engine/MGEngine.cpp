/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */
#include "MGEngine.h"
#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"
#include "Node/NodeIncludes.h"


juce_ImplementSingleton(RMPSettings);

ControllableContainer* getAppSettings();

MGEngine::MGEngine() :
	Engine(getApp().getApplicationName(), ".gyver")
	//defaultBehaviors("Test"),
	//ossiaFixture(nullptr)
{
	convertURL = "https://benjamin.kuperberg.fr/rulemapool/releases/convert.json";

	//Communication
	// OSCRemoteControl::getInstance()->addRemoteControlListener(UserInputManager::getInstance());
	//init here
	Engine::mainEngine = this;
	MGEngine::mainEngine = this;

	GlobalSettings::getInstance()->altScaleFactor->setDefaultValue(0.05);

	addChildControllableContainer(MediaManager::getInstance());
	addChildControllableContainer(ScreenManager::getInstance());

	ProjectSettings::getInstance()->addChildControllableContainer(RMPSettings::getInstance());
	ProjectSettings::getInstance()->addChildControllableContainer(AudioManager::getInstance());

	// MIDIManager::getInstance(); //Trigger constructor, declare settings

	// getAppSettings()->addChildControllableContainer(&defaultBehaviors);

	//NDIManager::getInstance();
}

MGEngine::~MGEngine()
{
	//Application-end cleanup, nothing should be recreated after this
	//delete singletons here

	isClearing = true;

	MediaManager::deleteInstance();
	ScreenManager::deleteInstance();
	NDIManager::deleteInstance();
#if !JUCE_LINUX
	WebcamManager::deleteInstance();
#endif
	RMPSettings::deleteInstance();
	MediaClipFactory::deleteInstance();
	CompositionLayerFactory::deleteInstance();
	NodeFactory::deleteInstance();
	MediaGridUIPreview::deleteInstance();

	AudioManager::deleteInstance();
}


void MGEngine::clearInternal()
{
	//clear
	ScreenManager::getInstance()->clear();
	MediaManager::getInstance()->clear();
}

var MGEngine::getJSONData(bool includeNonOverriden)
{
	var data = Engine::getJSONData(includeNonOverriden);

	var MediaData = MediaManager::getInstance()->getJSONData();
	if (!MediaData.isVoid() && MediaData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(MediaManager::getInstance()->shortName, MediaData);

	var ScreenData = ScreenManager::getInstance()->getJSONData();
	if (!ScreenData.isVoid() && ScreenData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(ScreenManager::getInstance()->shortName, ScreenData);


	return data;
}

void MGEngine::loadJSONDataInternalEngine(var data, ProgressTask* loadingTask)
{
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

void MGEngine::childStructureChanged(ControllableContainer* cc)
{
	Engine::childStructureChanged(cc);
}

void MGEngine::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	if (isClearing || isLoadingFile) return;
}

void MGEngine::handleAsyncUpdate()
{
	Engine::handleAsyncUpdate();
}

String MGEngine::getMinimumRequiredFileVersion()
{
	return "1.0.0b1";
}

void MGEngine::importSelection()
{
	const MessageManagerLock mmLock;
	FileChooser fc("Load a craft", File::getCurrentWorkingDirectory(), "*.craft");

	fc.launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& fc) {
		if (!fc.getResult().exists()) return;

		File f = fc.getResult();
		var data = JSON::parse(f.loadFileAsString());
		if (!data.isObject())
		{
			LOGERROR("File is not a valid craft, go to the workshop and fix it !");
			return;
		}
		importCraft(data);
		});
}

void MGEngine::importCraft(var data)
{
	if (!data.isObject()) return;

	ScreenManager::getInstance()->addItemsFromData(data.getProperty(ScreenManager::getInstance()->shortName, var()));
	MediaManager::getInstance()->addItemsFromData(data.getProperty(MediaManager::getInstance()->shortName, var()));
}

void MGEngine::exportSelection()
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty(ScreenManager::getInstance()->shortName, ScreenManager::getInstance()->getExportSelectionData());
	data.getDynamicObject()->setProperty(MediaManager::getInstance()->shortName, MediaManager::getInstance()->getExportSelectionData());

	String s = JSON::toString(data);


	FileChooser fc("Save a craft", File::getCurrentWorkingDirectory(), "*.craft");

	fc.launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles, [this, s](const FileChooser& fc) {
		File f = fc.getResult();
		if (f == File()) return;
		if (f.existsAsFile()) f.deleteFile();

		FileOutputStream fos(f);
		fos.writeString(s);
		fos.flush();

		});
}

void MGEngine::parameterValueChanged(Parameter* p) {
	Engine::parameterValueChanged(p);
	/* if (p == panelScale) {
		InputPanel::getInstance()->resized();
	}
	*/

}

RMPSettings::RMPSettings() :
	ControllableContainer("RMP Settings")
{
	fpsLimit = addIntParameter("FPS Limit", "Limit the framerate", 60, 0, 360);
	fpsLimit->canBeDisabledByUser = true;
}
