/*
  ==============================================================================

	VideoMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once



// Forward declaration
class VideoMediaAudioProcessor;

class VideoMedia :
	public Media,
	public MPVPlayer::MPVListener
{
public:
	VideoMedia(var params = var());
	~VideoMedia();

	enum VideoSource { Source_File, Source_URL };
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;

	enum PlayerState { UNLOADED, LOADING, PLAYING, PAUSED, STATES_MAX };
	const String playerStateNames[STATES_MAX] = { "Unloaded", "Loading", "Playing", "Paused" };
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

	bool updatingPosFromPlayer;
	bool manuallySeeking;
	uint32 timeAtLastSeek;

	int videoWidth = 0;
	int videoHeight = 0;


	std::unique_ptr<MPVPlayer> mpv;

	void clearItem() override;

	void setupMPV(const String& path);

	void load();

	void play();
	void stop();
	void pause();
	void restart();
	void seek(double time);


	void onContainerParameterChanged(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void initGLInternal() override;
	void renderOpenGL() override;
	void renderGLInternal() override;
	void closeGLInternal() override;

	virtual void handleEnter(double time, bool play = false) override;
	virtual void handleExit() override;
	virtual void handleSeek(double time) override;
	virtual void handleStop() override;
	virtual void handleStart() override;

	//MPVEvents
	void mpvFileLoaded() override;
	void mpvTimeChanged(double time) override;
	void mpvFrameUpdate() override;
	void mpvFileEnd() override;

	bool isPlaying();
	double getMediaLength() override;
	Point<int> getMediaSize(const String& texName = String()) override;
	String getMediaContentName() const override;

	void afterLoadJSONDataInternal() override;

	DECLARE_TYPE("Video")
};
