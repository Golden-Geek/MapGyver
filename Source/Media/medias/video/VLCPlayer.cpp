/*
  ==============================================================================

	VLCPlayer.cpp
	VLC-based video playback engine implementation

  ==============================================================================
*/

#include "VLCPlayer.h"

#ifdef VLC_ENABLE

// =============================================================================
// VLCTimers — shared 60 Hz singleton, same pattern as MPVTimers
// =============================================================================

class VLCTimers : public juce::Timer
{
public:
	juce_DeclareSingleton(VLCTimers, true);
	VLCTimers()  { startTimerHz(60); }
	~VLCTimers() { stopTimer(); players.clear(); }

	void registerVLC(VLCPlayer* p)   { players.addIfNotAlreadyThere(p); }
	void unregisterVLC(VLCPlayer* p) { players.removeAllInstancesOf(p); }

	void timerCallback() override
	{
		for (auto* p : players)
			p->pullEvents();
	}

	juce::Array<VLCPlayer*> players;
};
juce_ImplementSingleton(VLCTimers);

// =============================================================================
// VLCPlayer Implementation
// =============================================================================

VLCPlayer::VLCPlayer(const String& filePath)
	: filePath(filePath)
{
	setupVLC();
	VLCTimers::getInstance()->registerVLC(this);
}

VLCPlayer::~VLCPlayer()
{
	VLCTimers::getInstance()->unregisterVLC(this);
	unload();

	if (mediaPlayer)
	{
		libvlc_media_player_release(mediaPlayer);
		mediaPlayer = nullptr;
	}

	if (vlcInstance)
	{
		libvlc_release(vlcInstance);
		vlcInstance = nullptr;
	}
}

void VLCPlayer::setupVLC()
{
	const char* vlcArgs[] = {
		"--no-xlib",
		"--verbose=0",
		"--no-video-title-show",
		"--avcodec-hw=any",
		"--no-osd",                      // disables marq/logo overlays (also silences those option errors)
		"--no-spu",                      // no subtitle/SPU decoding
		"--no-stats",                    // no playback statistics
		"--no-metadata-network-access",  // no network requests for art/tags
		"--clock-jitter=0",              // zero clock jitter for tighter A/V sync
		"--drop-late-frames",            // drop frames that arrive past their deadline
		"--skip-frames",                 // allow frame-skipping to maintain sync
	};

	vlcInstance = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs);

	if (!vlcInstance)
	{
		DBG("Failed to create VLC instance");
		return;
	}

	mediaPlayer = libvlc_media_player_new(vlcInstance);

	if (!mediaPlayer)
	{
		DBG("Failed to create VLC media player");
		return;
	}

	// Setup video callbacks for frame capture
	libvlc_video_set_callbacks(mediaPlayer,
								vlcLockCallback,
								vlcUnlockCallback,
								vlcDisplayCallback,
								this);

	// "BGRA" byte layout matches JUCE Image::ARGB (B,G,R,A in memory), giving correct alpha=255.
	// Placeholder dimensions overridden in load() once the media is parsed.
	libvlc_video_set_format(mediaPlayer, "BGRA", 1920, 1080, 1920 * 4);

	// Setup event callbacks
	libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(mediaPlayer);

	libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying,       vlcEventCallback, this);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused,        vlcEventCallback, this);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped,       vlcEventCallback, this);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached,    vlcEventCallback, this);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerTimeChanged,   vlcEventCallback, this);
	libvlc_event_attach(eventManager, libvlc_MediaPlayerLengthChanged, vlcEventCallback, this);

	setupAudio();
}

void VLCPlayer::setupAudio()
{
	audioProcessor = std::make_unique<VLCAudioProcessor>();

	// Set audio callbacks
	libvlc_audio_set_callbacks(mediaPlayer,
								vlcAudioPlayCallback,
								nullptr,  // pause
								nullptr,  // resume
								vlcAudioFlushCallback,
								nullptr,  // drain
								this);

	// Set audio format to stereo float 48kHz
	libvlc_audio_set_format(mediaPlayer, "fl32", 48000, 2);
}

bool VLCPlayer::load(const juce::String& filePath)
{
	unload();

	currentFile = filePath;

	// Create media from file path
	currentMedia = libvlc_media_new_path(vlcInstance, filePath.toRawUTF8());

	if (!currentMedia)
	{
		DBG("VLC: Failed to create media from path: " + filePath);
		return false;
	}

	DBG("VLC: Loading file: " + filePath);

	// Set media to player
	libvlc_media_player_set_media(mediaPlayer, currentMedia);

	// Parse media to get duration and metadata (synchronous for now)
	libvlc_media_parse(currentMedia);

	// Wait a bit for parsing (this is blocking but simple)
	juce::Thread::sleep(100);

	// Get duration in milliseconds, convert to seconds
	libvlc_time_t dur = libvlc_media_get_duration(currentMedia);
	duration = dur / 1000.0;

	DBG("VLC: Duration = " + juce::String(duration.load()) + "s");

	// Read actual video dimensions from parsed track info
	unsigned vidW = 1920, vidH = 1080;
	libvlc_media_track_t** tracks = nullptr;
	unsigned numTracks = libvlc_media_tracks_get(currentMedia, &tracks);
	for (unsigned i = 0; i < numTracks; ++i)
	{
		if (tracks[i]->i_type == libvlc_track_video && tracks[i]->video->i_width > 0)
		{
			vidW = tracks[i]->video->i_width;
			vidH = tracks[i]->video->i_height;
			break;
		}
	}
	libvlc_media_tracks_release(tracks, numTracks);

	DBG("VLC: Video size = " + juce::String(vidW) + "x" + juce::String(vidH));

	// Set format with correct dimensions and allocate CPU frame buffer
	libvlc_video_set_format(mediaPlayer, "BGRA", vidW, vidH, vidW * 4);
	{
		juce::ScopedLock lock(videoLock);
		videoWidth  = (int)vidW;
		videoHeight = (int)vidH;
		videoBuffer.resize(vidW * vidH * 4);
		videoFrame  = juce::Image(juce::Image::ARGB, (int)vidW, (int)vidH, true);
	}

	isLoaded = true;

	// Notify listeners that file is loaded
	listeners.call([this](Listener& l) {
		DBG("VLC: Calling playerFileLoaded callback");
		l.playerFileLoaded();
	});

	DBG("VLC: Load complete, ready to play");

	return true;
}

void VLCPlayer::unload()
{
	stop();

	if (currentMedia)
	{
		libvlc_media_release(currentMedia);
		currentMedia = nullptr;
	}

	isLoaded = false;
	frameReady = false;
}

void VLCPlayer::play()
{
	if (!mediaPlayer || !isLoaded)
	{
		DBG("VLC: Cannot play - mediaPlayer=" + juce::String((int64)mediaPlayer) + " isLoaded=" + juce::String(isLoaded ? "true" : "false"));
		return;
	}

	DBG("VLC: Starting playback");
	int result = libvlc_media_player_play(mediaPlayer);
	if (result == 0)
	{
		DBG("VLC: Play command succeeded");
		playing = true;
	}
	else
	{
		DBG("VLC: Play command failed with code " + juce::String(result));
	}
}

void VLCPlayer::pause()
{
	if (!mediaPlayer) return;

	libvlc_media_player_pause(mediaPlayer);
	playing = false;
}

void VLCPlayer::stop()
{
	if (!mediaPlayer) return;

	libvlc_media_player_stop(mediaPlayer);
	playing = false;

	if (audioProcessor)
		audioProcessor->flush();
}

bool VLCPlayer::isPlaying() const
{
	return playing.load();
}

void VLCPlayer::setPosition(double pos)
{
	if (!mediaPlayer || !isLoaded) return;

	libvlc_time_t ms = (libvlc_time_t)(juce::jlimit(0.0, duration.load(), pos) * 1000.0);
	libvlc_media_player_set_time(mediaPlayer, ms);
	lastVLCTimeMs = ms;
	lastVLCWallMs = juce::Time::currentTimeMillis();
}

double VLCPlayer::getPosition() const
{
	if (!mediaPlayer || !isLoaded) return 0.0;

	return libvlc_media_player_get_time(mediaPlayer) / 1000.0; // Convert ms to seconds
}

double VLCPlayer::getDuration() const
{
	return duration.load();
}

void VLCPlayer::setPlaySpeed(float speed)
{
	if (!mediaPlayer) return;

	currentSpeed = speed;
	libvlc_media_player_set_rate(mediaPlayer, speed);
}

float VLCPlayer::getPlaySpeed() const
{
	return currentSpeed.load();
}

void VLCPlayer::setVolume(float volume)
{
	if (!mediaPlayer) return;

	currentVolume = volume;
	int vlcVolume = (int)(juce::jlimit(0.0f, 1.0f, volume) * 100.0f);
	libvlc_audio_set_volume(mediaPlayer, vlcVolume);
}

float VLCPlayer::getVolume() const
{
	return currentVolume.load();
}

void VLCPlayer::setLoop(bool loop)
{
	loopEnabled = loop;
}

bool VLCPlayer::getLoop() const
{
	return loopEnabled.load();
}

int VLCPlayer::getVideoWidth() const
{
	return videoWidth;
}

int VLCPlayer::getVideoHeight() const
{
	return videoHeight;
}

int VLCPlayer::getNumChannels() const
{
	return numAudioChannels.load();
}

void VLCPlayer::setupGL()
{
	// VLC handles video decoding internally, we just need to receive frames
}

void VLCPlayer::renderGL(int width, int height)
{
	if (!frameReady) return;

	juce::ScopedLock lock(videoLock);

	if (videoFrame.isValid())
	{
		// Draw the video frame
		juce::Graphics g(videoFrame);
		// The frame is already in videoFrame, OpenGL rendering would happen in the caller
	}
}

juce::AudioProcessor* VLCPlayer::getAudioProcessor()
{
	return audioProcessor.get();
}

void VLCPlayer::pullEvents()
{
	if (!isLoaded || !playing.load()) return;

	int64_t elapsed = juce::Time::currentTimeMillis() - lastVLCWallMs.load();
	double interpolated = lastVLCTimeMs.load() * 0.001 + elapsed * 0.001 * currentSpeed.load();
	interpolated = juce::jlimit(0.0, duration.load(), interpolated);
	listeners.call([interpolated](Listener& l) { l.playerTimeChanged(interpolated); });
}

// =============================================================================
// VLC Callbacks
// =============================================================================

void VLCPlayer::vlcEventCallback(const libvlc_event_t* event, void* userData)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(userData);
	if (!player) return;

	player->handleVLCEvent(event);
}

void VLCPlayer::handleVLCEvent(const libvlc_event_t* event)
{
	switch (event->type)
	{
		case libvlc_MediaPlayerPlaying:
			DBG("VLC Event: Playing");
			playing = true;
			lastVLCWallMs = juce::Time::currentTimeMillis(); // restart interpolation from now
			break;

		case libvlc_MediaPlayerPaused:
			DBG("VLC Event: Paused");
			playing = false;
			{
				libvlc_time_t t = libvlc_media_player_get_time(mediaPlayer);
				if (t >= 0) lastVLCTimeMs = t; // snap anchor to exact paused position
			}
			break;

		case libvlc_MediaPlayerStopped:
			DBG("VLC Event: Stopped");
			playing = false;
			break;

		case libvlc_MediaPlayerEndReached:
			DBG("VLC Event: End Reached");
			playing = false;
			listeners.call([this](Listener& l) { l.playerFileEnd(); });

			if (loopEnabled.load())
			{
				setPosition(0.0);
				play();
			}
			break;

		case libvlc_MediaPlayerTimeChanged:
			// Calibration tick: update interpolation anchor with the authoritative VLC time
			lastVLCTimeMs = event->u.media_player_time_changed.new_time;
			lastVLCWallMs = juce::Time::currentTimeMillis();
			break;

		case libvlc_MediaPlayerLengthChanged:
			duration = libvlc_media_player_get_length(mediaPlayer) / 1000.0;
			break;

		default:
			break;
	}
}

void* VLCPlayer::vlcLockCallback(void* opaque, void** planes)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(opaque);
	if (!player) return nullptr;

	player->videoLock.enter();

	*planes = player->videoBuffer.empty() ? nullptr : player->videoBuffer.data();
	return nullptr;
}


void VLCPlayer::vlcUnlockCallback(void* opaque, void* picture, void* const* planes)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(opaque);
	if (!player) return;

	// Copy buffer to JUCE image
	if (player->videoFrame.isValid())
	{
		juce::Image::BitmapData bitmap(player->videoFrame, juce::Image::BitmapData::writeOnly);
		memcpy(bitmap.data, player->videoBuffer.data(), player->videoBuffer.size());
	}

	player->videoLock.exit();
}

void VLCPlayer::vlcDisplayCallback(void* opaque, void* picture)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(opaque);
	if (!player) return;

	player->frameReady = true;
	player->listeners.call([player](Listener& l) { l.playerFrameUpdate(); });
}

juce::Image VLCPlayer::getVideoFrame() const
{
	juce::ScopedLock lock(videoLock);
	return videoFrame;
}

void VLCPlayer::vlcAudioPlayCallback(void* data, const void* samples, unsigned count, int64_t pts)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(data);
	if (!player || !player->audioProcessor) return;

	const float* floatSamples = static_cast<const float*>(samples);
	player->audioProcessor->pushAudioData(floatSamples, count, 2);
}

void VLCPlayer::vlcAudioFlushCallback(void* data, int64_t pts)
{
	VLCPlayer* player = static_cast<VLCPlayer*>(data);
	if (!player || !player->audioProcessor) return;

	player->audioProcessor->flush();
}

// =============================================================================
// VLCAudioProcessor Implementation
// =============================================================================

VLCAudioProcessor::VLCAudioProcessor()
{
	fifoBuffer.setSize(2, fifoSize);
	fifoBuffer.clear();
}

VLCAudioProcessor::~VLCAudioProcessor()
{
}

void VLCAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	fifoBuffer.clear();
	writePosition = 0;
	readPosition = 0;
	availableSamples = 0;
}

void VLCAudioProcessor::releaseResources()
{
}

void VLCAudioProcessor::pushAudioData(const float* samples, int numSamples, int numChannels)
{
	juce::ScopedLock lock(audioLock);

	for (int i = 0; i < numSamples; ++i)
	{
		if (availableSamples < fifoSize)
		{
			for (int ch = 0; ch < juce::jmin(2, numChannels); ++ch)
			{
				fifoBuffer.setSample(ch, writePosition, samples[i * numChannels + ch]);
			}

			writePosition = (writePosition + 1) % fifoSize;
			availableSamples++;
		}
	}
}

void VLCAudioProcessor::flush()
{
	juce::ScopedLock lock(audioLock);
	fifoBuffer.clear();
	writePosition = 0;
	readPosition = 0;
	availableSamples = 0;
}

void VLCAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedLock lock(audioLock);

	int numSamples = buffer.getNumSamples();
	int numChannels = juce::jmin(buffer.getNumChannels(), 2);

	for (int i = 0; i < numSamples; ++i)
	{
		if (availableSamples > 0)
		{
			for (int ch = 0; ch < numChannels; ++ch)
			{
				buffer.setSample(ch, i, fifoBuffer.getSample(ch, readPosition));
			}

			readPosition = (readPosition + 1) % fifoSize;
			availableSamples--;
		}
		else
		{
			// No audio available, output silence
			for (int ch = 0; ch < numChannels; ++ch)
			{
				buffer.setSample(ch, i, 0.0f);
			}
		}
	}
}

#endif // VLC_ENABLE
