/*
  ==============================================================================

    Main.h
    Created: 25 Oct 2016 11:16:59pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

//==============================================================================
class SpiderMapApplication : public OrganicApplication
{
public:
	//==============================================================================
	SpiderMapApplication();


	void initialiseInternal(const String& /*commandLine*/) override;
	void afterInit() override;

	void shutdown() override;

	void handleCrashed() override;
};

START_JUCE_APPLICATION(SpiderMapApplication)