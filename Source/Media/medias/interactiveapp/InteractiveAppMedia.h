/*
  ==============================================================================

	InteractiveAppMedia.h
	Created: 11 Feb 2025 3:01:59pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class InteractiveAppMedia :
	public BaseSharedTextureMedia,
	public Timer,
	public Thread,
	public SimpleWebSocketClient::Listener
{
public:
	InteractiveAppMedia(var params = var());
	~InteractiveAppMedia();

	FileParameter* appParam;
	StringParameter* launchArguments;
	EnumParameter* availableTextures;

	enum AppState { CLOSED, LAUNCHING, RUNNING, CLOSING };
	EnumParameter* appState;
	BoolParameter* autoStartOnPreUse;
	BoolParameter* autoStartOnUse;
	BoolParameter* autoStopOnUse;
	BoolParameter* stopOnClear;

	BoolParameter* hardKill;
	BoolParameter* launchMinimized;

	IntParameter* oscQueryPort;
	BoolParameter* keepValuesOnSync;
	BoolParameter* isConnected;
	StringParameter* serverName;

	std::unique_ptr<SimpleWebSocketClient> wsClient;
	OSCSender oscSender;

	bool checkingProcess;
	bool shouldMinimize;
	bool shouldSynchronize;

	var treeData;
	bool isUpdatingStructure;
	bool hasListenExtension;

	Array<Controllable*> noFeedbackList;


	void clearItem() override;

	void onContainerParameterChangedInternal(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void checkAppRunning();
	bool isAppRunning();
	void updateTextureList();

	void updateBeingUsed() override;


	void syncOSCQuery();
	void requestHostInfo();
	void requestStructure();
	void updateTreeFromData(var data);

	void setupWSAndOSC();

	void sendOSC(const OSCMessage& m);
	void sendOSCForControllable(Controllable* c);

	void updateAllListens();
	void updateListenToContainer(OSCQueryHelpers::OSCQueryValueContainer* gcc, bool onlySendIfListen = false);

	void connectionOpened() override;
	void connectionClosed(int status, const String& reason) override;
	void connectionError(const String& errorMessage) override;

	void dataReceived(const MemoryBlock& data) override;
	void processOSCMessage(const OSCMessage& m);
	void messageReceived(const String& message) override;

	static OSCArgument varToArgument(const var& v);


	void launchProcess();
	void killProcess();
	void timerCallback() override;

#if JUCE_WINDOWS
	static BOOL enumWindowCallback(HWND hWnd, LPARAM lparam);
#endif

	void run() override;

	String getMediaContentName() const override;

	var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataInternal(var data) override;

	DECLARE_TYPE("Interactive App")
};