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
	public VideoPlayerEngine,
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

	// VideoPlayerEngine interface implementation
	bool load(const String& filePath) override;
	void unload() override;
	void play() override;
	void pause() override;
	void stop() override;
	bool isPlaying() const override;
	void setPosition(double pos) override;
	double getPosition() const override;
	double getDuration() const override;
	void setPlaySpeed(float speed) override;
	float getPlaySpeed() const override;
	void setVolume(float volume) override;
	float getVolume() const override;
	void setLoop(bool loop) override;
	bool getLoop() const override;
	int getVideoWidth() const override;
	int getVideoHeight() const override;
	int getNumChannels() const override;
	void setupGL() override;
	void renderGL(int width, int height) override;
	AudioProcessor* getAudioProcessor() override;
	void pullEvents() override;

	// MPV-specific methods
	void loadFile();
	void stopGLUpdates();
	void clearGL();
	void renderGL(OpenGLFrameBuffer* frameBuffer);
	bool isGLInit();
	void setPlaySpeedInternal(double speed);
	bool isShutdownComplete() const { return shutdownComplete.load(); }
	void abandonForProcessExit();
	static void destroyAfterShutdown(std::unique_ptr<MPVPlayer> player);
	static void drainDeferredPlayers();
	static bool hasDeferredPlayers();


	// MPV Stuff
	void onMPVUpdate();
	void onMPVWakeup();

	// Internal state
	bool shouldLoop = false;
	float currentVolume = 1.0f;
	float currentSpeed = 1.0f;


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
#if JUCE_WINDOWS
		HANDLE pipeHandle = INVALID_HANDLE_VALUE;
#else
#endif
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
	double getMPVDoubleProperty(const char* name) const;
	String getMPVStringProperty(const char* name);

	struct FileInfo
	{
		bool fileLoaded = false;
		int width = 0;
		int height = 0;
		double duration = 0;
		int numChannels = 0;
	};
	FileInfo fileInfo;
	int pendingFileInfoMask = 0;
	bool eofReached = false;
	std::atomic<bool> playbackActive{ false };
	std::atomic<bool> shutdownRequested{ false };
	std::atomic<bool> shutdownCommandComplete{ false };
	std::atomic<bool> shutdownComplete{ false };

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
