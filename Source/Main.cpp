//==============================================================================

#if JUCE_WINDOWS
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "MainIncludes.h"
#include "Engine/MGEngine.h"
#include "Screen/ScreenIncludes.h"


MapGyverApplication::MapGyverApplication() :
	OrganicApplication("MapGyver", true, ImageCache::getFromMemory(BinaryData::icon_png, BinaryData::icon_pngSize))
{
}


void MapGyverApplication::initialiseInternal(const String&)
{
	engine.reset(new MGEngine());
	if (useWindow) mainComponent.reset(new MainContentComponent());

	//Call after engine init
	AppUpdater::getInstance()->setURLs("https://www.goldengeek.org/mapgyver/releases/update.json", "https://www.goldengeek.org/mapgyver/download/app", getApplicationName());
	// HelpBox::getInstance()->helpURL = URL("https://www.goldengeek.org/mapgyver/help/");

	//CrashDumpUploader::getInstance()->init("https://www.goldengeek.org/mapgyver/support/crash_report.php",ImageCache::getFromMemory(BinaryData::crash_png, BinaryData::crash_pngSize));

	DashboardManager::getInstance()->setupDownloadURL("https://benjamin.kuperberg.fr/download/dashboard/dashboard.php?folder=dashboard");

	ShapeShifterManager::getInstance()->setDefaultFileData(BinaryData::default_smlayout);
	ShapeShifterManager::getInstance()->setLayoutInformations("mglayout", getApplicationName() + "/layouts");
}


void MapGyverApplication::afterInit()
{
	//ANALYTICS
	if (mainWindow != nullptr)
	{

		// MainContentComponent* comp = (MainContentComponent*)mainComponent.get();
		// MGEngine* eng = (MGEngine*)engine.get();
		//RMPMenuBarComponent* menu = new RMPMenuBarComponent(comp, eng);
		// mainWindow->setMenuBarComponent(menu);
	}


}

void MapGyverApplication::systemRequestedQuit()
{
	if (shutdownPrepared)
	{
		quit();
		return;
	}

	if (Engine::mainEngine == nullptr)
	{
		quit();
		return;
	}

	Engine::mainEngine->saveIfNeededAndUserAgreesAsync([this](FileBasedDocument::SaveResult result)
		{
			switch (result)
			{
			case FileBasedDocument::SaveResult::userCancelledSave:
				return;

			case FileBasedDocument::SaveResult::failedToWriteToFile:
				LOGERROR("Could not save the document (Failed to write to file)\nCancelled closing of the application");
				return;

			case FileBasedDocument::SaveResult::savedOk:
				Engine::mainEngine->removeNewerAutosaves();
				break;
			}

			prepareShutdownAfterSave();
		});
}

void MapGyverApplication::prepareShutdownAfterSave()
{
	if (shutdownPrepared)
		return;

	shutdownPrepared = true;
	isShuttingDown = true;

	// This is intentionally done before JUCE's quit message stops normal message
	// dispatch. MPV and JUCE's GL render thread can then complete without either
	// side waiting on a message-manager lock held by the other.
	ScreenOutputWatcher::deleteInstance();
	if (engine != nullptr)
		engine->clear();

	quit();
}

void MapGyverApplication::shutdown()
{
	if (UltralightManager::getInstanceWithoutCreating()) UltralightManager::getInstance()->clear();

	// DocumentWindow destruction invalidates the cached OpenGL image and closes
	// GlContextHolder immediately. Clear media first so every VideoMedia can
	// unregister, stop MPV, and free its render context in the safe two-phase path.
	isShuttingDown = true;
	ScreenOutputWatcher::deleteInstance();
	if (engine != nullptr)
	{
		engine->clear();
		MPVPlayer::drainDeferredPlayers();
	}

	OrganicApplication::shutdown();

	// MPV's deferred GL work was drained above. Destroy the UI-owned OpenGL and
	// screen helpers now, while Engine::mainEngine and ScreenManager are still
	// valid; MainContentComponent owns ScreenOutputWatcher.
	mainComponent.reset();
	engine.reset();

	AppUpdater::deleteInstance();
	FileDownloader::deleteInstance();
}

void MapGyverApplication::handleCrashed()
{
	/*
	for (auto& m : ModuleManager::getInstance()->getItemsWithType<OSModule>())
	{
		m->crashedTrigger->trigger();
	}

	if (enableSendAnalytics->boolValue())
	{
		MatomoAnalytics::getInstance()->log(MatomoAnalytics::CRASH);
		while (MatomoAnalytics::getInstance()->isThreadRunning())
		{
			//wait until thread is done
		}
	}
	*/
	OrganicApplication::handleCrashed();
}
