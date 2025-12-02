/*
  ==============================================================================

	VideoMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

// Forward declaration
class VideoMediaAudioProcessor;

class VideoMedia :
	public Media,
	public AudioManager::AudioManagerListener,
	public Timer
{
public:
	VideoMedia(var params = var());
	~VideoMedia();

	void clearItem() override;

	enum VideoSource { Source_File, Source_URL };
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;

	enum PlayerState { UNLOADED, IDLE, READY, PLAYING, PAUSED, STATES_MAX };
	const String playerStateNames[STATES_MAX] = { "Unloaded", "Idle", "Ready", "Playing", "Paused" };
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

	mpv_handle* mpv = nullptr;
	mpv_render_context* mpv_gl = nullptr;

	bool updatingPosFromVLC;
	bool manuallySeeking;
	uint32 timeAtLastSeek;

	int videoWidth = 0;
	int videoHeight = 0;

	// Audio Processor Graph
	AudioProcessorGraph::NodeID audioNodeID;
	VideoMediaAudioProcessor* audioProcessor;
	int numAudioTracks = 0;

	class PipeThread : public Thread
	{
	public:
		PipeThread(VideoMedia* owner, String pipePath);
		~PipeThread() override;
		void run() override;
		void shutdown();

	private:
		VideoMedia* owner;
		String pipePath;
		HANDLE pipeHandle = INVALID_HANDLE_VALUE;
		std::vector<float> readBuffer;
	};

	std::unique_ptr<PipeThread> pipeThread;
	String uniquePipePath;
	// ==============================================================================

	void onContainerParameterChanged(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void setupAudio();
	void audioSetupChanged() override;

	void load();

	void initGLInternal() override;
	void renderGLInternal() override;
	void closeGLInternal() override;

	// MPV Stuff
	void onMPVUpdate();
	void onMPVWakeup();
	void pullEvents();

	int getMPVIntProperty(const char* name);
	double getMPVDoubleProperty(const char* name);
	String getMPVStringProperty(const char* name);

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

	bool isPlaying();
	double getMediaLength() override;
	Point<int> getMediaSize(const String& texName = String()) override;
	String getMediaContentName() const override;

	void afterLoadJSONDataInternal() override;

	void timerCallback() override;

	DECLARE_TYPE("Video")
};