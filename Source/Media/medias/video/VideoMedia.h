/*
  ==============================================================================

	VideoMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class VideoMedia :
	public ImageMedia
	//public Thread
{
public:
	VideoMedia(var params = var());
	~VideoMedia();

	enum VideoSource { Source_File, Source_URL };
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;

	enum PlayerState { UNLOADED, IDLE, PLAYING, PAUSED, STATES_MAX };
	const String playerStateNames[STATES_MAX] = { "Unloaded", "Idle", "Playing", "Paused" };
	EnumParameter* state;
	FloatParameter* position;
	FloatParameter* length;

	ControllableContainer controlsCC;
	Trigger* playTrigger;
	Trigger* stopTrigger;
	Trigger* pauseTrigger;
	Trigger* restartTrigger;
	BoolParameter* playAtLoad;
	BoolParameter* loop;
	FloatParameter* playSpeed;

	ControllableContainer audioCC;
	FloatParameter* volume;

	//BoolParameter* playAtLoad;
	//BoolParameter* loop;
	//Trigger* startBtn;
	//Trigger* stopBtn;
	//Trigger* restartBtn;
	//Trigger* pauseBtn;
	//FloatParameter* mediaVolume;
	//String currentVolumeController = "";
	//String nextVolumeController = "";
	//FloatParameter* speedRate;
	//FloatParameter* seek;
	//BoolParameter* usePreroll;




	VLC::Instance* vlcInstance;
	std::unique_ptr<VLC::MediaPlayer> vlcPlayer;
	std::unique_ptr<VLC::Media> vlcMedia;

	int imageWidth = 0;
	int imageHeight = 0;
	int imagePitches = 0;
	int imageLines = 0;

	double frameRate;
	double totalFrames;

	bool updatingPosFromVLC;
	bool isSeeking;

	//double lastTapTempo;
	//Trigger* tapTempoBtn;
	//IntParameter* beatPerCycle;


	//double frameRate = 0;
	//double timeAtLastEvent = 0;
	//double frameAtLastEvent = 0;

	//void clearItem() override;
	void onContainerParameterChanged(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;


	//void triggerTriggered(Trigger* t);

	void load();
	void play();
	void stop();
	void pause();
	void restart();
	void seek(double time);

	virtual void handleEnter(double time, bool play = false) override;
	virtual void handleExit() override;
	virtual void handleSeek(double time) override;
	virtual void handleStop() override;
	virtual void handleStart() override;
	
	void afterLoadJSONDataInternal() override;

	//void unload();



	//void run();

	//void tapTempo();

	DECLARE_TYPE("Video")
};