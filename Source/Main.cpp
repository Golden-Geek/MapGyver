//==============================================================================

#include "MainIncludes.h"
#include "Engine/SMEngine.h"

SpiderMapApplication::SpiderMapApplication() :
	OrganicApplication("SpiderMap", true, ImageCache::getFromMemory(BinaryData::icon_png, BinaryData::icon_pngSize))
{
}


void SpiderMapApplication::initialiseInternal(const String&)
{
	engine.reset(new SMEngine());
	if (useWindow) mainComponent.reset(new MainContentComponent());

	//Call after engine init
	AppUpdater::getInstance()->setURLs("https://benjamin.kuperberg.fr/rulemapool/releases/update.json", "https://benjamin.kuperberg.fr/rulemapool/download/app", getApplicationName());
	// HelpBox::getInstance()->helpURL = URL("https://benjamin.kuperberg.fr/rulemapool/help/");

	//CrashDumpUploader::getInstance()->init("https://benjamin.kuperberg.fr/rulemapool/support/crash_report.php",ImageCache::getFromMemory(BinaryData::crash_png, BinaryData::crash_pngSize));

	DashboardManager::getInstance()->setupDownloadURL("https://benjamin.kuperberg.fr/download/dashboard/dashboard.php?folder=dashboard");

	ShapeShifterManager::getInstance()->setDefaultFileData(BinaryData::default_smlayout);
	ShapeShifterManager::getInstance()->setLayoutInformations("smlayout", getApplicationName() + "/layouts");
}


void SpiderMapApplication::afterInit()
{
	//ANALYTICS
	if (mainWindow != nullptr)
	{

		// MainContentComponent* comp = (MainContentComponent*)mainComponent.get();
		// SMEngine* eng = (SMEngine*)engine.get();
		//RMPMenuBarComponent* menu = new RMPMenuBarComponent(comp, eng);
		// mainWindow->setMenuBarComponent(menu);
	}


}

void SpiderMapApplication::shutdown()
{
	OrganicApplication::shutdown();
	AppUpdater::deleteInstance();
}

void SpiderMapApplication::handleCrashed()
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
