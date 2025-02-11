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
	public Timer
{
public:
	InteractiveAppMedia(var params = var());
	~InteractiveAppMedia();

	FileParameter* appParam;
	StringParameter* launchArguments;
	EnumParameter* availableTextures;

	BoolParameter* appRunning;
	BoolParameter* autoStartOnPreUse;
	BoolParameter* autoStartOnUse;
	BoolParameter* autoStopOnUse;
	BoolParameter* hardKill;

	SimpleWebSocketClient oscQuery;

	bool checkingProcess;

	void onContainerParameterChangedInternal(Parameter* p) override;

	void checkAppRunning();
	void updateTextureList();

	void updateBeingUsed() override;

	void launchProcess();
	void killProcess();
	void timerCallback() override;

	DECLARE_TYPE("Interactive App")
};