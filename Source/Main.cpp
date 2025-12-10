//==============================================================================

#if JUCE_WINDOWS
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "MainIncludes.h"
#include "Engine/MGEngine.h"


MapGyverApplication::MapGyverApplication() :
	OrganicApplication("MapGyver", true, ImageCache::getFromMemory(BinaryData::icon_png, BinaryData::icon_pngSize))
{
}


void MapGyverApplication::initialiseInternal(const String&)
{
	engine.reset(new MGEngine());
	if (useWindow) mainComponent.reset(new MainContentComponent());

	//Call after engine init
	AppUpdater::getInstance()->setURLs("https://benjamin.kuperberg.fr/mapgyver/releases/update.json", "https://benjamin.kuperberg.fr/mapgyver/download/app", getApplicationName());
	// HelpBox::getInstance()->helpURL = URL("https://benjamin.kuperberg.fr/mapgyver/help/");

	//CrashDumpUploader::getInstance()->init("https://benjamin.kuperberg.fr/mapgyver/support/crash_report.php",ImageCache::getFromMemory(BinaryData::crash_png, BinaryData::crash_pngSize));

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

void MapGyverApplication::shutdown()
{
	if (UltralightManager::getInstanceWithoutCreating()) UltralightManager::getInstance()->clear();
	OrganicApplication::shutdown();
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
