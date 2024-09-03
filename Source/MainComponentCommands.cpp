/*
 ==============================================================================

 MainComponentMenuBar.cpp
 Created: 25 Mar 2016 6:02:02pm
 Author:  Martin Hermant

 ==============================================================================
 */
 #include "MainIncludes.h"

namespace SMCommandID
{
	static const int showAbout = 0x60000;
	static const int gotoWebsite = 0x60001;
	static const int gotoDiscord = 0x60002;
	static const int gotoDocs = 0x60003;
	static const int postGithubIssue = 0x60004;
	static const int donate = 0x60005;
	static const int sponsor = 0x60055;
	static const int showWelcome = 0x60006;
	static const int gotoChangelog = 0x60007;

	static const int guideStart = 0x300; //up to 0x300 +100
	static const int exitGuide = 0x399; 
	static const int goToCommunityModules = 0x500;
	static const int reloadCustomModules = 0x501;
	static const int exportSelection = 0x800;
	static const int importSelection = 0x801;

}

void MainContentComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) 
{
	if (commandID >= SMCommandID::guideStart && commandID < SMCommandID::guideStart + 99)
	{
		// result.setInfo(Guider::getInstance()->getGuideName(commandID - SMCommandID::guideStart), "", "Guides", result.readOnlyInKeyEditor);
		return;
	}

	switch (commandID)
	{
	case SMCommandID::showAbout:
		result.setInfo("About...", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::showWelcome:
		result.setInfo("Show Welcome Screen...", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::donate:
		result.setInfo("Be cool and donate", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::sponsor:
		result.setInfo("Be even cooler and sponsor !", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::gotoWebsite:
		result.setInfo("Go to website", "", "Help", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::gotoDiscord:
		result.setInfo("Go to Discord", "", "Help", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::gotoDocs:
		result.setInfo("Go to the Amazing Documentation", "", "Help", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::gotoChangelog:
		result.setInfo("See the changelog", "", "Help", result.readOnlyInKeyEditor);
		break;


	case SMCommandID::postGithubIssue:
		result.setInfo("Post an issue on github", "", "Help", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::goToCommunityModules:
		result.setInfo("Community Modules Manager", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::reloadCustomModules:
		result.setInfo("Reload Custom Modules", "", "General", result.readOnlyInKeyEditor);
		break;

	case SMCommandID::exitGuide:
		result.setInfo("Exit current guide", "", "Guides", result.readOnlyInKeyEditor);
		result.addDefaultKeypress(KeyPress::escapeKey, ModifierKeys::noModifiers);
		// result.setActive(Guider::getInstance()->guide != nullptr);
		break;

	case SMCommandID::exportSelection:
		result.setInfo("Export Selection", "This will export the current selection as *.mochi file that can be later imported", "File", result.readOnlyInKeyEditor);
		result.addDefaultKeypress(KeyPress::createFromDescription("s").getKeyCode(), ModifierKeys::altModifier);
		result.setActive(InspectableSelectionManager::mainSelectionManager->currentInspectables.size() > 0);
		break;

	case SMCommandID::importSelection:
		result.setInfo("Import...", "This will import a *.mochi file and add it to the current project", "File", result.readOnlyInKeyEditor);
		result.addDefaultKeypress(KeyPress::createFromDescription("o").getKeyCode(), ModifierKeys::altModifier);
		break;


	default:
		OrganicMainContentComponent::getCommandInfo(commandID, result);
		break;
	}
}



void MainContentComponent::getAllCommands(Array<CommandID>& commands) {

	OrganicMainContentComponent::getAllCommands(commands);
	const CommandID ids[] = {

		// SMCommandID::showAbout,
		//SMCommandID::showWelcome,
		//SMCommandID::donate,
		//SMCommandID::sponsor,
		//SMCommandID::gotoWebsite,
		//SMCommandID::gotoDiscord,
		//SMCommandID::gotoDocs,
		//SMCommandID::gotoChangelog,
		//SMCommandID::postGithubIssue,
		SMCommandID::importSelection,
		SMCommandID::exportSelection


	};

	commands.addArray(ids, numElementsInArray(ids));
	//for (int i = 0; i < Guider::getInstance()->factory.defs.size(); ++i) commands.add(SMCommandID::guideStart + i);
}


PopupMenu MainContentComponent::getMenuForIndex(int topLevelMenuIndex, const String& menuName) 
{
	PopupMenu menu = OrganicMainContentComponent::getMenuForIndex(topLevelMenuIndex, menuName);

	if (menuName == "Help")
	{
		menu.addCommandItem(&getCommandManager(), SMCommandID::showAbout);
		menu.addCommandItem(&getCommandManager(), SMCommandID::showWelcome);
		menu.addCommandItem(&getCommandManager(), SMCommandID::donate);
		menu.addCommandItem(&getCommandManager(), SMCommandID::sponsor);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), SMCommandID::gotoWebsite);
		menu.addCommandItem(&getCommandManager(), SMCommandID::gotoDiscord);
		menu.addCommandItem(&getCommandManager(), SMCommandID::gotoDocs);
		menu.addCommandItem(&getCommandManager(), SMCommandID::gotoChangelog);
		menu.addCommandItem(&getCommandManager(), SMCommandID::postGithubIssue);

	}else if (menuName == "Guides")
	{
	/*
		for (int i = 0; i < Guider::getInstance()->factory.defs.size(); ++i)
		{
			menu.addCommandItem(&getCommandManager(), SMCommandID::guideStart + i);
		}

		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), SMCommandID::exitGuide);
	*/
	}

	return menu;
}

void MainContentComponent::fillFileMenuInternal(PopupMenu & menu)
{
	menu.addCommandItem(&getCommandManager(), SMCommandID::importSelection);
	menu.addCommandItem(&getCommandManager(), SMCommandID::exportSelection);
	//menu.addSeparator();
	//menu.addCommandItem(&getCommandManager(), SMCommandID::goToCommunityModules);
	//menu.addCommandItem(&getCommandManager(), SMCommandID::reloadCustomModules);
}

bool MainContentComponent::perform(const InvocationInfo& info)
{

	if (info.commandID >= SMCommandID::guideStart && info.commandID < SMCommandID::guideStart + 100)
	{
		// Guider::getInstance()->launchGuideAtIndex(info.commandID - SMCommandID::guideStart);
		return true;
	}

	switch (info.commandID)
	{


	case SMCommandID::showAbout:
	{
		// AboutWindow w;
		// DialogWindow::showModalDialog("About", &w, getTopLevelComponent(), Colours::transparentBlack, true);
	}
	break;

	case SMCommandID::showWelcome:
	{
		// WelcomeScreen w;
		// DialogWindow::showModalDialog("Welcome", &w, getTopLevelComponent(), Colours::black, true);
	}
	break;

	case SMCommandID::donate:
		URL("https://benjamin.kuperberg.fr/spidermap/").launchInDefaultBrowser();
		break;

	case SMCommandID::sponsor:
		URL("https://benjamin.kuperberg.fr/spidermap/").launchInDefaultBrowser();
		break;

	case SMCommandID::gotoWebsite:
		URL("https://benjamin.kuperberg.fr/spidermap/").launchInDefaultBrowser();
		break;

	case SMCommandID::gotoDiscord:
		URL("https://discord.gg/wYNB3rK").launchInDefaultBrowser();
		break;

	case SMCommandID::gotoDocs:
		URL("https://benjamin.kuperberg.fr/spidermap/docs").launchInDefaultBrowser();
		break;

	case SMCommandID::gotoChangelog:
		URL("https://benjamin.kuperberg.fr/spidermap/releases/changelog.html").launchInDefaultBrowser();
		break;

	case SMCommandID::postGithubIssue:
		URL("http://github.com/benkuper/spidermap/issues").launchInDefaultBrowser();
		break;


	case SMCommandID::goToCommunityModules:
		// CommunityModuleManager::getInstance()->selectThis();
		break;

	case SMCommandID::reloadCustomModules:
		// ModuleManager::getInstance()->factory->updateCustomModules();
		break;

	case SMCommandID::exitGuide:
		// Guider::getInstance()->setCurrentGuide(nullptr);
		break;

	case SMCommandID::exportSelection:
	{
		((SMEngine*)Engine::mainEngine)->exportSelection();
	}
	break;

	case SMCommandID::importSelection:
	{
		((SMEngine*)Engine::mainEngine)->importSelection();
	}
	break;
	/*
	case SMCommandID::keyAssistant: {Assistant::getInstance()->selectThis(); }break;

	case SMCommandID::keyFixture: {	UserInputManager::getInstance()->processInput("Fixture"); }break;
	case SMCommandID::keyGroup: {	UserInputManager::getInstance()->processInput("Group"); }break;
	case SMCommandID::keyPreset: {	UserInputManager::getInstance()->processInput("Preset"); }break;
	case SMCommandID::keyCuelist: {	UserInputManager::getInstance()->processInput("Cuelist"); }break;
	case SMCommandID::keyCue: {		UserInputManager::getInstance()->processInput("Cue"); }break;
	case SMCommandID::keyEffect: {	UserInputManager::getInstance()->processInput("Effect"); }break;
	case SMCommandID::keyCarousel: {	UserInputManager::getInstance()->processInput("Carousel"); }break;
	case SMCommandID::keyMapper: {	UserInputManager::getInstance()->processInput("Mapper"); }break;

	case SMCommandID::keyHighlight: {UserInputManager::getInstance()->toggleHightlight(); }break;
	case SMCommandID::keyBlind: {	UserInputManager::getInstance()->toggleBlind(); }break;

	case SMCommandID::keyClear: {	UserInputManager::getInstance()->processInput("Clear"); }break;
	case SMCommandID::keyBackspace: {UserInputManager::getInstance()->processInput("Backspace"); }break;
	case SMCommandID::keyEnter: {	UserInputManager::getInstance()->processInput("Enter"); }break;
	case SMCommandID::keyThru: {		UserInputManager::getInstance()->processInput("Thru"); }break;
	case SMCommandID::keyPlus: {		UserInputManager::getInstance()->processInput("+"); }break;
	case SMCommandID::keyMinus: {	UserInputManager::getInstance()->processInput("-"); }break;

	case SMCommandID::keyRecord: {	UserInputManager::getInstance()->processInput("record"); }break;
	case SMCommandID::keyCopy: {		UserInputManager::getInstance()->processInput("copy"); }break;
	case SMCommandID::keyEdit: {		UserInputManager::getInstance()->processInput("edit"); }break;
	case SMCommandID::keyDelete: {	UserInputManager::getInstance()->processInput("delete"); }break;

	case SMCommandID::key1: {		UserInputManager::getInstance()->processInput("1"); }break;
	case SMCommandID::key2: {		UserInputManager::getInstance()->processInput("2"); }break;
	case SMCommandID::key3: {		UserInputManager::getInstance()->processInput("3"); }break;
	case SMCommandID::key4: {		UserInputManager::getInstance()->processInput("4"); }break;
	case SMCommandID::key5: {		UserInputManager::getInstance()->processInput("5"); }break;
	case SMCommandID::key6: {		UserInputManager::getInstance()->processInput("6"); }break;
	case SMCommandID::key7: {		UserInputManager::getInstance()->processInput("7"); }break;
	case SMCommandID::key8: {		UserInputManager::getInstance()->processInput("8"); }break;
	case SMCommandID::key9: {		UserInputManager::getInstance()->processInput("9"); }break;
	case SMCommandID::key0: {		UserInputManager::getInstance()->processInput("0"); }break;
	*/

	default:
		return OrganicMainContentComponent::perform(info);
	}

	return true;
}

StringArray MainContentComponent::getMenuBarNames()
{
	StringArray names = OrganicMainContentComponent::getMenuBarNames();
	// names.add("Help");
	// names.add("Yataaa");
	return names;
}
