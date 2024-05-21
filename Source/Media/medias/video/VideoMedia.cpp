/*
  ==============================================================================

	VideoMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/RMPEngine.h"
#include "VideoMedia.h"

VideoMedia::VideoMedia(var params) :
	ImageMedia(getTypeString(), params),
	Thread("VLC frame checker")
{
	source = addEnumParameter("Source", "Source");
	source->addOption("File", Source_File)->addOption("URL", Source_URL);

	filePath = addFileParameter("File path", "File path", "");
	filePath->setAutoReload(true);

	url = addStringParameter("URL", "URL", "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", false);

	loop = addBoolParameter("Loop", "Loop video", false);
	playAtLoad = addBoolParameter("Play at load", "Play at load", false);

	startBtn = addTrigger("start", "");
	stopBtn = addTrigger("stop", "");
	restartBtn = addTrigger("restart", "");
	pauseBtn = addTrigger("pause", "");

	mediaVolume = addFloatParameter("Volume", "Media volume", 1, 0, 1);
	seek = addFloatParameter("Seek", "Manual seek", 1, 0, 1);
	seek->defaultUI = FloatParameter::TIME;
	seek->isSavable = false;

	speedRate = addFloatParameter("Speed rate", "Speed factor of video", 1, 0);
	beatPerCycle = addIntParameter("Beat by cycles", "Number of tap tempo beats by cycle", 1, 1);
	tapTempoBtn = addTrigger("Tap tempo", "");

	RMPEngine* e = dynamic_cast<RMPEngine*>(Engine::mainEngine);
	VLCInstance = e->VLCInstance;
	frameUpdated = false;

	customFPSTick = true;
}

VideoMedia::~VideoMedia()
{
	stop();
	if (VLCMediaListPlayer != nullptr) libvlc_media_list_player_release(VLCMediaListPlayer); VLCMediaListPlayer = nullptr;
	if (VLCMediaList != nullptr) libvlc_media_list_release(VLCMediaList); VLCMediaList = nullptr;
	if (VLCMediaPlayer != nullptr)
	{
		libvlc_event_detach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPositionChanged, vlcSeek, this);
		libvlc_media_player_release(VLCMediaPlayer); VLCMediaPlayer = nullptr;
	}

	if (VLCMedia != nullptr) libvlc_media_release(VLCMedia); VLCMedia = nullptr;
	stopThread(100);
}

void VideoMedia::clearItem()
{
	BaseItem::clearItem();
}

void VideoMedia::onContainerParameterChanged(Parameter* p)
{
	// LIBVLC_API int libvlc_audio_set_volume( libvlc_media_player_t *p_mi, int i_volume );

	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
		stop();
	}

	if (p == source || p == filePath || p == url)
	{
		setup();

	}
	else if (p == loop)
	{
		//if (loop->boolValue()) {
		//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_loop);
		//}
		//else {
		//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_default);
		//}

	}
	else if (p == mediaVolume)
	{
		currentVolumeController = nextVolumeController;
		nextVolumeController = "";
		int v = mediaVolume->floatValue() * 100;
		libvlc_audio_set_volume(VLCMediaPlayer, v);
	}
	else if (p == speedRate)
	{
		libvlc_media_player_set_rate(VLCMediaPlayer, speedRate->floatValue());
	}
	else if (p == seek)
	{
		if (!vlcSeekedLast && videoTotalTime > 0)
		{
			libvlc_media_player_set_position(VLCMediaPlayer, seek->doubleValue() / videoTotalTime);
		}
		vlcSeekedLast = false;
	}
	else if (p == playAtLoad)
	{
		if (playAtLoad->boolValue()) {
			restart();
		}
	}
}

void VideoMedia::triggerTriggered(Trigger* t)
{
	if (t == startBtn)  play();
	else if (t == stopBtn) stop();
	else if (t == restartBtn) restart();
	else if (t == pauseBtn) pause();
	else if (t == tapTempoBtn)tapTempo();
}

void VideoMedia::afterLoadJSONDataInternal()
{
	Media::afterLoadJSONDataInternal();
}

void VideoMedia::setup()
{
	stop();

	File f = filePath->getFile();
	if (!f.existsAsFile())
	{
		NLOGWARNING(niceName, "File not found : " << f.getFullPathName());
		return;
	}

	VLCMediaList = libvlc_media_list_new(VLCInstance);

	VideoSource s = source->getValueDataAsEnum<VideoSource>();


	if (s == Source_File) VLCMedia = libvlc_media_new_path(VLCInstance, filePath->getFile().getFullPathName().toRawUTF8());
	else VLCMedia = libvlc_media_new_location(VLCInstance, url->stringValue().toRawUTF8());

	libvlc_media_list_add_media(VLCMediaList, VLCMedia);

	VLCMediaPlayer = libvlc_media_player_new(VLCInstance); //libvlc_media_player_new_from_media(VLCMedia);
	libvlc_video_set_format_callbacks(VLCMediaPlayer, setup_video, cleanup_video);
	libvlc_video_set_callbacks(VLCMediaPlayer, lock, unlock, display, this);

	VLCMediaListPlayer = libvlc_media_list_player_new(VLCInstance);

	libvlc_media_list_player_set_media_list(VLCMediaListPlayer, VLCMediaList);
	libvlc_media_list_player_set_media_player(VLCMediaListPlayer, VLCMediaPlayer);
	//if (loop->boolValue()) {
	//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_loop);
	//}
	//else {
	//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_default);
	//}

	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPlaying, vlcPlaying, this);
	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerStopped, vlcStopped, this);
	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPaused, vlcPaused, this);
	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerEndReached, vlcEndReached, this);
	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPositionChanged, vlcSeek, this);
	libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerTimeChanged, vlcTimeChanged, this);

	libvlc_media_player_play(VLCMediaPlayer);


	// Parse the media to get metadata
	libvlc_media_parse_with_options(VLCMedia, libvlc_media_parse_local, -1);

	// Wait until the media is parsed
	while (libvlc_media_get_parsed_status(VLCMedia) != libvlc_media_parsed_status_done) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	libvlc_media_track_t** tracks;
	int numTracks = libvlc_media_tracks_get(VLCMedia, &tracks);

	for (int i = 0; i < numTracks; ++i) {
		if (tracks[i]->i_type == libvlc_track_video) {
			libvlc_video_track_t* videoTrack = tracks[i]->video;
			frameRate = static_cast<double>(videoTrack->i_frame_rate_num) / videoTrack->i_frame_rate_den;
		}
	}

	// Release track information
	libvlc_media_tracks_release(tracks, numTracks);

	libvlc_media_release(VLCMedia); VLCMedia = nullptr;
	if (playAtLoad->boolValue()) restart();
	else prepareFirstFrame();
}

void VideoMedia::prepareFirstFrame()
{
	isPrerolling = true;
	play();
}

void VideoMedia::play()
{
	if (VLCMediaPlayer != nullptr) {
		libvlc_media_list_player_play(VLCMediaListPlayer);
	}
}

void VideoMedia::stop()
{
	//if (VLCMediaPlayer != nullptr) {
	//	libvlc_media_list_player_stop(VLCMediaListPlayer);
	//}

	pause();
	seek->setValue(0);
}

void VideoMedia::pause()
{
	if (VLCMediaPlayer == nullptr) return;
	stopThread(100);
	if (libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing) libvlc_media_list_player_pause(VLCMediaListPlayer);

}

void VideoMedia::restart()
{
	seek->setValue(0);
	play();
	seek->setValue(0);
}

void VideoMedia::run()
{
	//evaluate current frame from last frame set and time, and if reaching the frame before the last and in loop mode, restart	

	while (!threadShouldExit())
	{
		wait(10);

		if (VLCMediaPlayer == nullptr) return;
		if (frameAtLastEvent == 0 || timeAtLastEvent == 0) continue;
		double currentTime = Time::getMillisecondCounterHiRes();
		double timeSinceLastSet = currentTime - timeAtLastEvent;
		double currentFrame = frameAtLastEvent + timeSinceLastSet * frameRate / 1000.0;
		double totalFrame = videoTotalTime * frameRate;
		if (currentFrame >= totalFrame - 2)
		{
			if (loop->boolValue())
			{
				seek->setValue(0);
			}
		}

	}

	frameAtLastEvent = 0;
	timeAtLastEvent = 0;
}


void* VideoMedia::lock(void** pixels)
{
	imageLock.enter();
	//pixels[0] = vlcData;
	pixels[0] = bitmapData->getLinePointer(0);



	return 0;
}

void VideoMedia::unlock(void* oldBuffer, void* const* pixels)
{
	imageLock.exit();
	shouldRedraw = true;

	if (isPrerolling) {
		pause();
		isPrerolling = false;
	}

	FPSTick();
}


void VideoMedia::display(void* nextBuffer)
{
	//LOG("display");


}


unsigned VideoMedia::setup_video(char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines)
{
	imageWidth = *width;
	imageHeight = *height;
	imagePitches = *pitches;
	imageLines = *lines;

	//GenericScopedLock lock(imageLock);

	initImage(imageWidth, imageHeight);

	vlcDataIsValid = true;
	memcpy(chroma, "RV32", 4);
	(*pitches) = imageWidth * 4;
	(*lines) = imageHeight;

	videoTotalTime = libvlc_media_player_get_length(VLCMediaPlayer) / 1000.0;
	seek->setRange(0, videoTotalTime);



	return 1;
}

void VideoMedia::cleanup_video()
{
	vlcDataIsValid = false;
}

void VideoMedia::vlcSeek()
{
	float pos = libvlc_media_player_get_position(VLCMediaPlayer);
	vlcSeekedLast = true;
	seek->setNormalizedValue(pos);

	//LOG("Time changed : " << libvlc_media_player_get_time(VLCMediaPlayer));
	timeAtLastEvent = Time::getMillisecondCounterHiRes();
	frameAtLastEvent = pos * videoTotalTime * frameRate;
}

void VideoMedia::vlcTimeChanged()
{

}

void VideoMedia::vlcEndReached()
{
	if (loop->boolValue())
	{
		restart();
	}
	else prepareFirstFrame();
}

void VideoMedia::vlcPlaying()
{
	//LOG("start play");
	if (loop->boolValue()) startThread();
}

void VideoMedia::vlcStopped()
{
	//LOG("stop play");
	stopThread(1000);
}

void VideoMedia::vlcPaused()
{
	//LOG("pause play");
	stopThread(1000);
}

void VideoMedia::tapTempo()
{
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTapTempo;
	lastTapTempo = now;
	if (delta < 3000) {
		delta = delta * (int)beatPerCycle->getValue();
		if (videoTotalTime > 0)
		{
			double rate = videoTotalTime / delta;
			speedRate->setValue(rate);
		}

		//speed->setValue(cpm);
	}
}

void VideoMedia::handleEnter(double time)
{

	float tPos = jlimit(0., 1., time / videoTotalTime);
	if (VLCMediaPlayer != nullptr && videoTotalTime > 0) libvlc_media_player_set_position(VLCMediaPlayer, tPos);
}

void VideoMedia::handleExit()
{
	stop();
}

void VideoMedia::handleSeek(double time)
{
	float tPos = time / videoTotalTime;
	if (loop->boolValue()) tPos = fmod(tPos, 1);
	else tPos = jlimit(0.f, 1.f, tPos);

	if (VLCMediaPlayer != nullptr && videoTotalTime > 0) libvlc_media_player_set_position(VLCMediaPlayer, tPos);
}

void VideoMedia::handleStop()
{
	pause();
}

void VideoMedia::handleStart()
{
	isPrerolling = false;
	play();
}
