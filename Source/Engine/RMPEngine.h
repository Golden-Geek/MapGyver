/*
 ==============================================================================

 ChataigneEngine.h
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */


#pragma once

#include "Media/MediaIncludes.h"

class RMPSettings :
	public ControllableContainer
{
public:
	juce_DeclareSingleton(RMPSettings, true);
	RMPSettings();
	~RMPSettings() {};

	IntParameter* fpsLimit;
};

class RMPEngine :
	public Engine
{
public:
	RMPEngine();
	~RMPEngine();


	std::unique_ptr<VLC::Instance> vlcInstance = nullptr;

	void clearInternal() override;

	var getJSONData() override;
	void loadJSONDataInternalEngine(var data, ProgressTask* loadingTask) override;

	void childStructureChanged(ControllableContainer* cc) override;
	void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	void handleAsyncUpdate() override;

	String getMinimumRequiredFileVersion() override;

	void importSelection();
	void exportSelection();
	void importMochi(var data);

	void parameterValueChanged(Parameter* p);

};

