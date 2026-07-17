/*
  ==============================================================================

    Main.h
    Created: 25 Oct 2016 11:16:59pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

//==============================================================================
class MapGyverApplication : public OrganicApplication
{
public:
	//==============================================================================
	MapGyverApplication();


	void initialiseInternal(const String& /*commandLine*/) override;
	void afterInit() override;

	void shutdown() override;
	void systemRequestedQuit() override;

	void handleCrashed() override;

private:
	bool shutdownPrepared = false;
	void prepareShutdownAfterSave();
};
START_JUCE_APPLICATION(MapGyverApplication)
