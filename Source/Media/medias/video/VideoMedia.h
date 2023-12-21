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
{
public:
	VideoMedia(var params = var());
	~VideoMedia();

	bool frameUpdated;

	enum VideoSource { Source_File, Source_URL };
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;

	BoolParameter* playAtLoad;
	Trigger* startBtn;
	Trigger* stopBtn;
	Trigger* restartBtn;
	Trigger* pauseBtn;
	FloatParameter* mediaVolume;
	String currentVolumeController = "";
	String nextVolumeController = "";
	FloatParameter* speedRate;
	FloatParameter* seek;

	libvlc_instance_t* VLCInstance = nullptr;
	libvlc_media_player_t* VLCMediaPlayer = nullptr;
	libvlc_media_list_player_t* VLCMediaListPlayer = nullptr;
	libvlc_media_list_t* VLCMediaList = nullptr;
	libvlc_media_t* VLCMedia = nullptr;

	int imageWidth = 0;
	int imageHeight = 0;
	int imagePitches = 0;
	int imageLines = 0;
	//uint32_t* vlcData;

	double videoTotalTime = 0;

	bool vlcDataIsValid = false;

	bool vlcSeekedLast = false;

	double lastTapTempo;
	Trigger* tapTempoBtn;
	IntParameter* beatPerCycle;

	void clearItem() override;
	void onContainerParameterChanged(Parameter* p) override;
	void triggerTriggered(Trigger* t);

	void afterLoadJSONDataInternal() override;

	void play();
	void stop();
	void pause();
	void restart();

	void* lock(void** pixels);
	static void* lock(void* self, void** pixels) { return static_cast<VideoMedia*>(self)->lock(pixels); };

	void unlock(void* oldBuffer, void* const* pixels);
	static void unlock(void* self, void* oldBuffer, void* const* pixels) { static_cast<VideoMedia*>(self)->unlock(oldBuffer, pixels); };

	void display(void* nextBuffer);
	static void display(void* self, void* nextBuffer) { static_cast<VideoMedia*>(self)->display(nextBuffer); };

	unsigned setup_video(char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines);
	static unsigned setup_video(void** self, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines) {
		return static_cast<VideoMedia*>(*self)->setup_video(chroma, width, height, pitches, lines);
	}

	void cleanup_video();
	static void cleanup_video(void* self) {
		static_cast<VideoMedia*>(self)->cleanup_video();
	}

	void vlcSeek();
	static void vlcSeek(const struct libvlc_event_t* p_event, void* p_data) {
		static_cast<VideoMedia*>(p_data)->vlcSeek();
	}
	//virtual MediaUI* createUI() {return new VideoMedia(); };

	
	void tapTempo();

	virtual void handleEnter(double time) override; 
	virtual void handleExit() override;
	virtual void handleSeek(double time) override;
	virtual void handleStop() override;
	virtual void handleStart() override;

	DECLARE_TYPE("Video")
};