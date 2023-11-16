/*
 ==============================================================================

 MainComponentMenuBar.cpp
 Created: 25 Mar 2016 6:02:02pm
 Author:  Martin Hermant

 ==============================================================================
 */
 #include "MainIncludes.h"

namespace RMPCommandId
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
	if (commandID >= RMPCommandId::guideStart && commandID < RMPCommandId::guideStart + 99)
	{
		// result.setInfo(Guider::getInstance()->getGuideName(commandID - RMPCommandId::guideStart), "", "Guides", result.readOnlyInKeyEditor);
		return;
	}

	switch (commandID)
	{
	case RMPCommandId::showAbout:
		result.setInfo("About...", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::showWelcome:
		result.setInfo("Show Welcome Screen...", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::donate:
		result.setInfo("Be cool and donate", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::sponsor:
		result.setInfo("Be even cooler and sponsor !", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::gotoWebsite:
		result.setInfo("Go to website", "", "Help", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::gotoDiscord:
		result.setInfo("Go to Discord", "", "Help", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::gotoDocs:
		result.setInfo("Go to the Amazing Documentation", "", "Help", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::gotoChangelog:
		result.setInfo("See the changelog", "", "Help", result.readOnlyInKeyEditor);
		break;


	case RMPCommandId::postGithubIssue:
		result.setInfo("Post an issue on github", "", "Help", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::goToCommunityModules:
		result.setInfo("Community Modules Manager", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::reloadCustomModules:
		result.setInfo("Reload Custom Modules", "", "General", result.readOnlyInKeyEditor);
		break;

	case RMPCommandId::exitGuide:
		result.setInfo("Exit current guide", "", "Guides", result.readOnlyInKeyEditor);
		result.addDefaultKeypress(KeyPress::escapeKey, ModifierKeys::noModifiers);
		// result.setActive(Guider::getInstance()->guide != nullptr);
		break;

	case RMPCommandId::exportSelection:
		result.setInfo("Export Selection", "This will export the current selection as *.mochi file that can be later imported", "File", result.readOnlyInKeyEditor);
		result.addDefaultKeypress(KeyPress::createFromDescription("s").getKeyCode(), ModifierKeys::altModifier);
		result.setActive(InspectableSelectionManager::mainSelectionManager->currentInspectables.size() > 0);
		break;

	case RMPCommandId::importSelection:
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

		// RMPCommandId::showAbout,
		//RMPCommandId::showWelcome,
		//RMPCommandId::donate,
		//RMPCommandId::sponsor,
		//RMPCommandId::gotoWebsite,
		//RMPCommandId::gotoDiscord,
		//RMPCommandId::gotoDocs,
		//RMPCommandId::gotoChangelog,
		//RMPCommandId::postGithubIssue,
		RMPCommandId::importSelection,
		RMPCommandId::exportSelection


	};

	commands.addArray(ids, numElementsInArray(ids));
	//for (int i = 0; i < Guider::getInstance()->factory.defs.size(); ++i) commands.add(RMPCommandId::guideStart + i);
}


PopupMenu MainContentComponent::getMenuForIndex(int topLevelMenuIndex, const String& menuName) 
{
	PopupMenu menu = OrganicMainContentComponent::getMenuForIndex(topLevelMenuIndex, menuName);

	if (menuName == "Help")
	{
		menu.addCommandItem(&getCommandManager(), RMPCommandId::showAbout);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::showWelcome);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::donate);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::sponsor);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), RMPCommandId::gotoWebsite);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::gotoDiscord);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::gotoDocs);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::gotoChangelog);
		menu.addCommandItem(&getCommandManager(), RMPCommandId::postGithubIssue);

	}else if (menuName == "Guides")
	{
	/*
		for (int i = 0; i < Guider::getInstance()->factory.defs.size(); ++i)
		{
			menu.addCommandItem(&getCommandManager(), RMPCommandId::guideStart + i);
		}

		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), RMPCommandId::exitGuide);
	*/
	}

	return menu;
}

void MainContentComponent::fillFileMenuInternal(PopupMenu & menu)
{
	menu.addCommandItem(&getCommandManager(), RMPCommandId::importSelection);
	menu.addCommandItem(&getCommandManager(), RMPCommandId::exportSelection);
	//menu.addSeparator();
	//menu.addCommandItem(&getCommandManager(), RMPCommandId::goToCommunityModules);
	//menu.addCommandItem(&getCommandManager(), RMPCommandId::reloadCustomModules);
}

bool MainContentComponent::perform(const InvocationInfo& info)
{

	if (info.commandID >= RMPCommandId::guideStart && info.commandID < RMPCommandId::guideStart + 100)
	{
		// Guider::getInstance()->launchGuideAtIndex(info.commandID - RMPCommandId::guideStart);
		return true;
	}

	switch (info.commandID)
	{


	case RMPCommandId::showAbout:
	{
		// AboutWindow w;
		// DialogWindow::showModalDialog("About", &w, getTopLevelComponent(), Colours::transparentBlack, true);
	}
	break;

	case RMPCommandId::showWelcome:
	{
		// WelcomeScreen w;
		// DialogWindow::showModalDialog("Welcome", &w, getTopLevelComponent(), Colours::black, true);
	}
	break;

	case RMPCommandId::donate:
		URL("http://RuleMapPool.lighting/").launchInDefaultBrowser();
		break;

	case RMPCommandId::sponsor:
		URL("http://RuleMapPool.lighting/").launchInDefaultBrowser();
		break;

	case RMPCommandId::gotoWebsite:
		URL("http://RuleMapPool.lighting/").launchInDefaultBrowser();
		break;

	case RMPCommandId::gotoDiscord:
		URL("https://discord.gg/wYNB3rK").launchInDefaultBrowser();
		break;

	case RMPCommandId::gotoDocs:
		URL("https://benjamin.kuperberg.fr/chataigne/docs").launchInDefaultBrowser();
		break;

	case RMPCommandId::gotoChangelog:
		URL("https://benjamin.kuperberg.fr/chataigne/releases/changelog.html").launchInDefaultBrowser();
		break;

	case RMPCommandId::postGithubIssue:
		URL("http://github.com/benkuper/Chataigne/issues").launchInDefaultBrowser();
		break;


	case RMPCommandId::goToCommunityModules:
		// CommunityModuleManager::getInstance()->selectThis();
		break;

	case RMPCommandId::reloadCustomModules:
		// ModuleManager::getInstance()->factory->updateCustomModules();
		break;

	case RMPCommandId::exitGuide:
		// Guider::getInstance()->setCurrentGuide(nullptr);
		break;

	case RMPCommandId::exportSelection:
	{
		((RMPEngine*)Engine::mainEngine)->exportSelection();
	}
	break;

	case RMPCommandId::importSelection:
	{
		((RMPEngine*)Engine::mainEngine)->importSelection();
	}
	break;
	/*
	case RMPCommandId::keyAssistant: {Assistant::getInstance()->selectThis(); }break;

	case RMPCommandId::keyFixture: {	UserInputManager::getInstance()->processInput("Fixture"); }break;
	case RMPCommandId::keyGroup: {	UserInputManager::getInstance()->processInput("Group"); }break;
	case RMPCommandId::keyPreset: {	UserInputManager::getInstance()->processInput("Preset"); }break;
	case RMPCommandId::keyCuelist: {	UserInputManager::getInstance()->processInput("Cuelist"); }break;
	case RMPCommandId::keyCue: {		UserInputManager::getInstance()->processInput("Cue"); }break;
	case RMPCommandId::keyEffect: {	UserInputManager::getInstance()->processInput("Effect"); }break;
	case RMPCommandId::keyCarousel: {	UserInputManager::getInstance()->processInput("Carousel"); }break;
	case RMPCommandId::keyMapper: {	UserInputManager::getInstance()->processInput("Mapper"); }break;

	case RMPCommandId::keyHighlight: {UserInputManager::getInstance()->toggleHightlight(); }break;
	case RMPCommandId::keyBlind: {	UserInputManager::getInstance()->toggleBlind(); }break;

	case RMPCommandId::keyClear: {	UserInputManager::getInstance()->processInput("Clear"); }break;
	case RMPCommandId::keyBackspace: {UserInputManager::getInstance()->processInput("Backspace"); }break;
	case RMPCommandId::keyEnter: {	UserInputManager::getInstance()->processInput("Enter"); }break;
	case RMPCommandId::keyThru: {		UserInputManager::getInstance()->processInput("Thru"); }break;
	case RMPCommandId::keyPlus: {		UserInputManager::getInstance()->processInput("+"); }break;
	case RMPCommandId::keyMinus: {	UserInputManager::getInstance()->processInput("-"); }break;

	case RMPCommandId::keyRecord: {	UserInputManager::getInstance()->processInput("record"); }break;
	case RMPCommandId::keyCopy: {		UserInputManager::getInstance()->processInput("copy"); }break;
	case RMPCommandId::keyEdit: {		UserInputManager::getInstance()->processInput("edit"); }break;
	case RMPCommandId::keyDelete: {	UserInputManager::getInstance()->processInput("delete"); }break;

	case RMPCommandId::key1: {		UserInputManager::getInstance()->processInput("1"); }break;
	case RMPCommandId::key2: {		UserInputManager::getInstance()->processInput("2"); }break;
	case RMPCommandId::key3: {		UserInputManager::getInstance()->processInput("3"); }break;
	case RMPCommandId::key4: {		UserInputManager::getInstance()->processInput("4"); }break;
	case RMPCommandId::key5: {		UserInputManager::getInstance()->processInput("5"); }break;
	case RMPCommandId::key6: {		UserInputManager::getInstance()->processInput("6"); }break;
	case RMPCommandId::key7: {		UserInputManager::getInstance()->processInput("7"); }break;
	case RMPCommandId::key8: {		UserInputManager::getInstance()->processInput("8"); }break;
	case RMPCommandId::key9: {		UserInputManager::getInstance()->processInput("9"); }break;
	case RMPCommandId::key0: {		UserInputManager::getInstance()->processInput("0"); }break;
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
