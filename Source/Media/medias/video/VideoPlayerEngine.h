/*
  ==============================================================================

	VideoPlayerEngine.h
	Abstract base interface for video playback engines (MPV, VLC, etc.)

  ==============================================================================
*/

#pragma once

class VideoPlayerEngine
{
public:
	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void playerFileLoaded() = 0;
		virtual void playerTimeChanged(double time) = 0;
		virtual void playerFrameUpdate() = 0;
		virtual void playerFileEnd() = 0;
	};

	VideoPlayerEngine() {}
	virtual ~VideoPlayerEngine() {}

	// Lifecycle
	virtual bool load(const juce::String& filePath) = 0;
	virtual void unload() = 0;

	// Playback control
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	virtual bool isPlaying() const = 0;

	// Seeking & position
	virtual void setPosition(double pos) = 0;
	virtual double getPosition() const = 0;
	virtual double getDuration() const = 0;

	// Playback parameters
	virtual void setPlaySpeed(float speed) = 0;
	virtual float getPlaySpeed() const = 0;
	virtual void setVolume(float volume) = 0;
	virtual float getVolume() const = 0;
	virtual void setLoop(bool loop) = 0;
	virtual bool getLoop() const = 0;

	// Video properties
	virtual int getVideoWidth() const = 0;
	virtual int getVideoHeight() const = 0;
	virtual int getNumChannels() const = 0;

	// Rendering
	virtual void setupGL() = 0;
	virtual void renderGL(int width, int height) = 0;

	// Audio
	virtual juce::AudioProcessor* getAudioProcessor() = 0;

	// Event polling (for engines that need it)
	virtual void pullEvents() {}

	// Listener management
	void addListener(Listener* listener) { listeners.add(listener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }

protected:
	juce::ListenerList<Listener> listeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoPlayerEngine)
};
