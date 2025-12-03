/*
  ==============================================================================

	MPVPlayer.h
	Created: 3 Dec 2025 9:44:00am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MPVPlayer;

class MPVTimers :
	public Timer
{
public:
	juce_DeclareSingleton(MPVTimers, true);
	MPVTimers();
	~MPVTimers();

	Array<MPVPlayer*> players;

	void registerMPV(MPVPlayer* vm);
	void unregisterMPV(MPVPlayer* vm);

	void timerCallback() override;
};

class MPVPlayer :
	public AudioManager::AudioManagerListener
{
public:
	MPVPlayer(const String& filePath);
	~MPVPlayer();

	String filePath;

	mpv_handle* mpv = nullptr;
	mpv_render_context* mpv_gl = nullptr;

	int currentPos = 0;

	void clear();
	void setupMPV();

	void loadFile();
	void setupGL();
	void renderGL(OpenGLFrameBuffer* frameBuffer);
	bool isGLInit();


	// MPV Stuff
	void onMPVUpdate();
	void onMPVWakeup();
	void pullEvents();

	// Controls
	void play();
	void pause();
	void stop();
	void setPosition(double pos);
	void setPlaySpeed(double speed);
	void setVolume(float volume);


	//Audio
	class AudioPipeThread : public Thread
	{
	public:
		AudioPipeThread(MPVPlayer* owner, String pipePath);
		~AudioPipeThread() override;
		void run() override;
		void shutdown();

	private:
		bool connected;
		MPVPlayer* owner;
		String pipePath;
		HANDLE pipeHandle = INVALID_HANDLE_VALUE;
		std::vector<float> readBuffer;
	};

	std::unique_ptr<AudioPipeThread> pipeThread;
	String uniquePipePath;

	// Audio Processor Graph
	AudioProcessorGraph::NodeID audioNodeID;
	MPVAudioProcessor* audioProcessor;

	void setupAudio();
	void audioSetupChanged() override;

	//Helpers
	int getMPVIntProperty(const char* name);
	double getMPVDoubleProperty(const char* name);
	String getMPVStringProperty(const char* name);

	int getVideoWidth();
	int getVideoHeight();
	double getDuration();
	int getNumChannels();

	bool isPlaying();

	struct FileInfo
	{
		bool fileLoaded = false;
		int width = 0;
		int height = 0;
		double duration = 0;
		int numChannels = 0;
	};
	FileInfo fileInfo;

	class MPVListener
	{
	public:
		//virtual void onMPVEvent(const String& eventName, var eventData) {}
		virtual void mpvFileLoaded() {}
		virtual void mpvFrameUpdate() {}
		virtual void mpvTimeChanged(double time) {}
		virtual void mpvFileEnd() {}
		virtual void mpvAudioSetupChanged() {}
	};

	LightweightListenerList<MPVListener> mpvListeners;
	void addMPVListener(MPVListener* listener) { mpvListeners.add(listener); }
	void removeMPVListener(MPVListener* listener) { mpvListeners.remove(listener); }
};