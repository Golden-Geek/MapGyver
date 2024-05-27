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
	controlsCC("Controls"),
	audioCC("Audio"),
	updatingPosFromVLC(false),
	isSeeking(false)
	//Thread("VLC frame checker")
{

	//init vlc
	vlcInstance = dynamic_cast<RMPEngine*>(Engine::mainEngine)->vlcInstance.get();
	vlcPlayer.reset(new VLC::MediaPlayer(*vlcInstance));
	vlcPlayer->play();


	source = addEnumParameter("Source", "Source");
	source->addOption("File", Source_File)->addOption("URL", Source_URL);

	filePath = addFileParameter("File path", "File path", "");
	filePath->setAutoReload(true);

	url = addStringParameter("URL", "URL", "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", false);

	state = addEnumParameter("State", "Player state");
	for (int i = 0; i < STATES_MAX; i++) state->addOption(playerStateNames[i], (PlayerState)i);
	state->setControllableFeedbackOnly(true);

	position = addFloatParameter("Time", "Time of video", 0, 0, 10);
	length = addFloatParameter("Length", "Length of video", 10, 0);
	length->setControllableFeedbackOnly(true);

	playTrigger = controlsCC.addTrigger("Play", "Play video");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop video");
	pauseTrigger = controlsCC.addTrigger("Pause", "Pause video");
	restartTrigger = controlsCC.addTrigger("Restart", "Restart video");
	playAtLoad = controlsCC.addBoolParameter("Play at load", "Play as soon as the video is loaded", false);
	loop = controlsCC.addBoolParameter("Loop", "Loop video", false);
	playSpeed = controlsCC.addFloatParameter("Speed", "Speed of video", 1, 0);


	volume = audioCC.addFloatParameter("Volume", "Volume of video", 1, 0, 1);

	addChildControllableContainer(&controlsCC);
	addChildControllableContainer(&audioCC);

	//beatPerCycle = addIntParameter("Beat by cycles", "Number of tap tempo beats by cycle", 1, 1);
	//tapTempoBtn = addTrigger("Tap tempo", "");

	//usePreroll = addBoolParameter("Use Preroll", "If checked, the video will be prerolled before playing", false);

	customFPSTick = true;
}

VideoMedia::~VideoMedia()
{
	//stop();
	//stopThread(100);
}

//void VideoMedia::clearItem()
//{
//	BaseItem::clearItem();
//}
//
void VideoMedia::onContainerParameterChanged(Parameter* p)
{
	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
		//stop();
	}

	else if (p == source || p == filePath || p == url)
	{
		load();
	}

	else if (p == position)
	{
		if (!updatingPosFromVLC) seek(position->doubleValue());
	}
}


void VideoMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	Media::onControllableFeedbackUpdateInternal(cc, c);

	if (cc == &controlsCC)
	{
		if (c == playTrigger) vlcPlayer->play();
		else if (c == stopTrigger) vlcPlayer->stopAsync();
		else if (c == pauseTrigger) vlcPlayer->pause();
		else if (c == restartTrigger) vlcPlayer->setPosition(0, true);
		else if (c == playSpeed) vlcPlayer->setRate(playSpeed->floatValue());
	}
	else if (cc == &audioCC)
	{
		if (c == volume) vlcPlayer->setVolume(volume->floatValue());
	}
}

void VideoMedia::load()
{
	//stop();

	File f = filePath->getFile();
	if (!f.existsAsFile())
	{
		NLOGWARNING(niceName, "File not found : " << f.getFullPathName());
		state->setValueWithData(IDLE);
		return;
	}

	vlcMedia.reset(new VLC::Media(f.getFullPathName().toStdString(), VLC::Media::FromType::FromPath));
	vlcPlayer.reset(new VLC::MediaPlayer(*vlcInstance, *vlcMedia));



	vlcPlayer->setVideoFormatCallbacks(
		[this](char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines) -> unsigned {

			GenericScopedLock lock(imageLock);
			imageWidth = *width;
			imageHeight = *height;
			imagePitches = *pitches;
			imageLines = *lines;

			initImage(imageWidth, imageHeight);

			//vlcDataIsValid = true;
			memcpy(chroma, "BGRA", 4);
			(*pitches) = imageWidth * 4;
			(*lines) = imageHeight;


			length->setValue(vlcPlayer->length() / 1000.0);
			position->setValue(0);
			position->setRange(0, length->doubleValue());

			std::vector<VLC::MediaTrack> tracks = vlcPlayer->tracks(VLC::MediaTrack::Video, false);
			if (tracks.size() > 0)
			{
				VLC::MediaTrack track = tracks[0];
				frameRate = track.fpsNum() * 1.0 / track.fpsDen();
				totalFrames = floor(length->doubleValue() * frameRate);
			}

			imageLock.exit();

			return 1;
		},
		[this]() {
			//imageLock.enter();
			//vlcDataIsValid = false;
			//imageLock.exit();
		});

	vlcPlayer->setVideoCallbacks(
		[this](void** data) -> void* {
			imageLock.enter();
			data[0] = bitmapData->getLinePointer(0);
			return nullptr;
		},
		[this](void* oldBuffer, void* const* pixels) {
			imageLock.exit();

			updatingPosFromVLC = true;

			if (!isSeeking)
			{
				position->setValue(vlcPlayer->time() / 1000.0);

				double currentFrame = round(position->doubleValue() * frameRate);
				if (currentFrame >= totalFrames - 2)
				{
					if (loop->boolValue()) vlcPlayer->setPosition(0, true);
				}
				updatingPosFromVLC = false;
			}
		},
		[this](void* data) {
			shouldRedraw = true;
			FPSTick();

		});

	state->setValueWithData(IDLE);

	if (!isCurrentlyLoadingData && playAtLoad->boolValue()) vlcPlayer->play();
}

void VideoMedia::play() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();

	if (st == IDLE || st == PAUSED)
	{
		vlcPlayer->play();
		state->setValueWithData(PlayerState::PLAYING);
	}
}

void VideoMedia::stop() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED)
	{
		vlcPlayer->stopAsync();
		state->setValueWithData(PlayerState::IDLE);
	}
}

void VideoMedia::pause() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING)
	{
		vlcPlayer->pause();
		state->setValueWithData(PlayerState::PAUSED);
	}
}

void VideoMedia::restart() {
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED || st == IDLE)
	{
		vlcPlayer->setPosition(0, true);
		vlcPlayer->play();
		state->setValueWithData(PlayerState::PLAYING);
	}
}

void VideoMedia::seek(double time)
{
	if (vlcPlayer == nullptr) return;
	PlayerState st = state->getValueDataAsEnum<PlayerState>();
	if (st == PLAYING || st == PAUSED)
	{
		double targetTime = time;
		bool isEnd = false;
		if (loop->boolValue()) targetTime = fmod(targetTime, length->doubleValue());
		double maxTime = length->doubleValue() - 1.0 / frameRate;
		targetTime = jlimit(0., maxTime, targetTime);
		if (targetTime >= maxTime) isEnd = true;

		isSeeking = true;
		vlcPlayer->setTime(targetTime * 1000, true);
		isSeeking = false;

		if (!isEnd && st == PLAYING && vlcPlayer->state() != libvlc_Playing) vlcPlayer->play();
	}
}


void VideoMedia::handleEnter(double time, bool doPlay)
{
	Media::handleEnter(time, doPlay);

	if (vlcPlayer == nullptr) return;

	seek(time);

	bool isEnd = false;
	if (position->doubleValue() >= length->doubleValue() - 1.0 / frameRate) isEnd = true;

	if (doPlay && !isEnd) play();
}

void VideoMedia::handleExit()
{
	Media::handleExit();
	stop();
}

void VideoMedia::handleSeek(double time)
{
	Media::handleSeek(time);
	seek(time);
}

void VideoMedia::handleStop()
{
	pause();
}

void VideoMedia::handleStart()
{
	play();
}

double VideoMedia::getMediaLength()
{
	return length->doubleValue();
}

void VideoMedia::afterLoadJSONDataInternal()
{
	Media::afterLoadJSONDataInternal();
	if (playAtLoad->boolValue()) play();
}




//	// LIBVLC_API int libvlc_audio_set_volume( libvlc_media_player_t *p_mi, int i_volume );
//
//	if (p == source)
//	{
//		bool isFile = source->getValueDataAsEnum<VideoSource>() == Source_File;
//		filePath->setEnabled(isFile);
//		url->setEnabled(!isFile);
//		stop();
//	}
//
//	if (p == source || p == filePath || p == url)
//	{
//		setup();
//
//	}
//	else if (p == loop)
//	{
//		//if (loop->boolValue()) {
//		//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_loop);
//		//}
//		//else {
//		//	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_default);
//		//}
//
//	}
//	else if (p == mediaVolume)
//	{
//		currentVolumeController = nextVolumeController;
//		nextVolumeController = "";
//		int v = mediaVolume->floatValue() * 100;
//		libvlc_audio_set_volume(VLCMediaPlayer, v);
//	}
//	else if (p == speedRate)
//	{
//		libvlc_media_player_set_rate(VLCMediaPlayer, speedRate->floatValue());
//	}
//	else if (p == seek)
//	{
//		if (!vlcSeekedLast && videoTotalTime > 0)
//		{
//			//libvlc_media_player_set_position(VLCMediaPlayer, seek->doubleValue() / videoTotalTime);
//		}
//		vlcSeekedLast = false;
//	}
//	else if (p == playAtLoad)
//	{
//		if (playAtLoad->boolValue()) {
//			restart();
//		}
//	}
//}
//
//void VideoMedia::triggerTriggered(Trigger* t)
//{
//	if (t == startBtn)  play();
//	else if (t == stopBtn) stop();
//	else if (t == restartBtn) restart();
//	else if (t == pauseBtn) pause();
//	else if (t == tapTempoBtn)tapTempo();
//}
//
//void VideoMedia::afterLoadJSONDataInternal()
//{
//	Media::afterLoadJSONDataInternal();
//}
//
//void VideoMedia::setup()
//{
//	//stop();
//
//	//File f = filePath->getFile();
//	//if (!f.existsAsFile())
//	//{
//	//	NLOGWARNING(niceName, "File not found : " << f.getFullPathName());
//	//	return;
//	//}
//
//	//VLCMediaList = libvlc_media_list_new(VLCInstance);
//
//	//VideoSource s = source->getValueDataAsEnum<VideoSource>();
//
//
//	//if (s == Source_File) VLCMedia = libvlc_media_new_path(VLCInstance, filePath->getFile().getFullPathName().toRawUTF8());
//	//else VLCMedia = libvlc_media_new_location(VLCInstance, url->stringValue().toRawUTF8());
//
//	//libvlc_media_list_add_media(VLCMediaList, VLCMedia);
//
//	//VLCMediaPlayer = libvlc_media_player_new(VLCInstance); //libvlc_media_player_new_from_media(VLCMedia);
//	//libvlc_video_set_format_callbacks(VLCMediaPlayer, setup_video, cleanup_video);
//	//libvlc_video_set_callbacks(VLCMediaPlayer, lock, unlock, display, this);
//
//	//VLCMediaListPlayer = libvlc_media_list_player_new(VLCInstance);
//
//	//libvlc_media_list_player_set_media_list(VLCMediaListPlayer, VLCMediaList);
//	//libvlc_media_list_player_set_media_player(VLCMediaListPlayer, VLCMediaPlayer);
//	////if (loop->boolValue()) {
//	////	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_loop);
//	////}
//	////else {
//	////	libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_default);
//	////}
//
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPlaying, vlcPlaying, this);
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerStopped, vlcStopped, this);
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPaused, vlcPaused, this);
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerEndReached, vlcEndReached, this);
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPositionChanged, vlcSeek, this);
//	//libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerTimeChanged, vlcTimeChanged, this);
//
//	//libvlc_media_player_play(VLCMediaPlayer);
//
//
//	//// Parse the media to get metadata
//	//libvlc_media_parse_with_options(VLCMedia, libvlc_media_parse_local, -1);
//
//	//// Wait until the media is parsed
//	//while (libvlc_media_get_parsed_status(VLCMedia) != libvlc_media_parsed_status_done) {
//	//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
//	//}
//
//	//libvlc_media_track_t** tracks;
//	//int numTracks = libvlc_media_tracks_get(VLCMedia, &tracks);
//
//	//for (int i = 0; i < numTracks; ++i) {
//	//	if (tracks[i]->i_type == libvlc_track_video) {
//	//		libvlc_video_track_t* videoTrack = tracks[i]->video;
//	//		frameRate = static_cast<double>(videoTrack->i_frame_rate_num) / videoTrack->i_frame_rate_den;
//	//	}
//	//}
//
//	//// Release track information
//	//libvlc_media_tracks_release(tracks, numTracks);
//
//	//libvlc_media_release(VLCMedia); VLCMedia = nullptr;
//	//if (playAtLoad->boolValue()) restart();
//	//else if (usePreroll->boolValue()) prepareFirstFrame();
//}
//
//void VideoMedia::prepareFirstFrame()
//{
//	isPrerolling = true;
//	play();
//}
//
//void VideoMedia::play()
//{
//	//LOG("play");
//	if (VLCMediaPlayer != nullptr) {
//		libvlc_media_list_player_play(VLCMediaListPlayer);
//	}
//}
//
//void VideoMedia::stop()
//{
//	if (isStopping) return;
//	//LOG("stop");
//	if (usePreroll->boolValue())
//	{
//		pause();
//		seek->setValue(0);
//	}
//	else
//	{
//		bool isPlaying = VLCMediaPlayer != nullptr && libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//		if (isPlaying) {
//			//LOG("Stop here");
//			GenericScopedLock lock(imageLock);
//			//LOG("After Lock");
//			//isStopping = true;
//			// 
//			// TODO : find a way to stop the player without crashing
//			//libvlc_media_list_player_stop(VLCMediaListPlayer);
//			// 
//			//isStopping = false;
//			//LOG("After stop");
//		}
//	}
//
//
//}
//
//void VideoMedia::pause()
//{
//	if (VLCMediaPlayer == nullptr) return;
//	//LOG("pause");
//	stopThread(100);
//	if (libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing) libvlc_media_list_player_pause(VLCMediaListPlayer);
//
//}
//
//void VideoMedia::restart()
//{
//	seek->setValue(0);
//	play();
//	seek->setValue(0);
//}
//
//void VideoMedia::run()
//{
//	//evaluate current frame from last frame set and time, and if reaching the frame before the last and in loop mode, restart	
//
//	while (!threadShouldExit())
//	{
//		wait(10);
//
//		if (VLCMediaPlayer == nullptr) return;
//		if (frameAtLastEvent == 0 || timeAtLastEvent == 0) continue;
//		double currentTime = Time::getMillisecondCounterHiRes();
//		double timeSinceLastSet = currentTime - timeAtLastEvent;
//		double currentFrame = frameAtLastEvent + timeSinceLastSet * frameRate / 1000.0;
//		double totalFrame = videoTotalTime * frameRate;
//		if (currentFrame >= totalFrame - 2)
//		{
//			if (loop->boolValue())
//			{
//				seek->setValue(0);
//			}
//		}
//
//	}
//
//	frameAtLastEvent = 0;
//	timeAtLastEvent = 0;
//}
//
//

//
//
//unsigned VideoMedia::setup_video(char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines)
//{
//	imageWidth = *width;
//	imageHeight = *height;
//	imagePitches = *pitches;
//	imageLines = *lines;
//
//	//GenericScopedLock lock(imageLock);
//
//	initImage(imageWidth, imageHeight);
//
//	vlcDataIsValid = true;
//	memcpy(chroma, "RV32", 4);
//	(*pitches) = imageWidth * 4;
//	(*lines) = imageHeight;
//
//	videoTotalTime = libvlc_media_player_get_length(VLCMediaPlayer) / 1000.0;
//	seek->setRange(0, videoTotalTime);
//
//
//
//	return 1;
//}
//
//void VideoMedia::cleanup_video()
//{
//	vlcDataIsValid = false;
//}
//
//void VideoMedia::vlcSeek()
//{
//	float pos = libvlc_media_player_get_position(VLCMediaPlayer);
//	vlcSeekedLast = true;
//	seek->setNormalizedValue(pos);
//
//	//LOG("Time changed : " << libvlc_media_player_get_time(VLCMediaPlayer));
//	timeAtLastEvent = Time::getMillisecondCounterHiRes();
//	frameAtLastEvent = pos * videoTotalTime * frameRate;
//}
//
//void VideoMedia::vlcTimeChanged()
//{
//
//}
//
//void VideoMedia::vlcEndReached()
//{
//	if (loop->boolValue())
//	{
//		restart();
//	}
//	else if (usePreroll->boolValue()) prepareFirstFrame();
//	else
//	{
//		stop();
//	}
//}
//
//void VideoMedia::vlcPlaying()
//{
//	//LOG("vlc start play");
//	if (loop->boolValue()) startThread();
//}
//
//void VideoMedia::vlcStopped()
//{
//	//LOG("vlc stop play");
//	stopThread(1000);
//}
//
//void VideoMedia::vlcPaused()
//{
//	//LOG("vlc pause play");
//	stopThread(1000);
//}
//
//void VideoMedia::tapTempo()
//{
//	double now = Time::getMillisecondCounterHiRes();
//	double delta = now - lastTapTempo;
//	lastTapTempo = now;
//	if (delta < 3000) {
//		delta = delta * (int)beatPerCycle->getValue();
//		if (videoTotalTime > 0)
//		{
//			double rate = videoTotalTime / delta;
//			speedRate->setValue(rate);
//		}
//
//		//speed->setValue(cpm);
//	}
//}
//
//void VideoMedia::handleEnter(double time)
//{
//	if (VLCMediaPlayer == nullptr) return;
//	
//	
//	bool isPlaying = VLCMediaPlayer != nullptr && libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//	//LOG("Handle enter " << (int)isPlaying);
//
//	float tPos = jlimit(0., 1., time / videoTotalTime);
//	//stop();
//	//prepareFirstFrame();
//	if (VLCMediaPlayer != nullptr && videoTotalTime > 0)
//	{
//		seek->setNormalizedValue(tPos);
//		//libvlc_media_player_set_position(VLCMediaPlayer, tPos);
//	}
//}
//
//void VideoMedia::handleExit()
//{
//	if (VLCMediaPlayer == nullptr) return;
//	bool isPlaying = libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//	//LOG("Handle exit " << (int)isPlaying);
//	stop();
//}
//
//void VideoMedia::handleSeek(double time)
//{
//
//
//	float tPos = time / videoTotalTime;
//	if (loop->boolValue()) tPos = fmod(tPos, 1);
//	else tPos = jlimit(0.f, 1.f, tPos);
//
//	if (VLCMediaPlayer != nullptr && videoTotalTime > 0)
//	{
//		bool isPlaying = libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//		//LOG("Handle seek " << (int)isPlaying);
//
//		//if(!isPlaying) play();
//		pause();
//
//		seek->setNormalizedValue(tPos);
//		if (isPlaying) libvlc_media_player_play(VLCMediaPlayer);
//
//
//	}
//}
//
//void VideoMedia::handleStop()
//{
//	if (VLCMediaPlayer == nullptr) return;
//	bool isPlaying = libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//	//LOG("Handle stop " << (int)isPlaying);
//	pause();
//}
//
//void VideoMedia::handleStart()
//{
//	if (VLCMediaPlayer == nullptr) return;
//	bool isPlaying = libvlc_media_player_get_state(VLCMediaPlayer) == libvlc_Playing;
//	//LOG("Handle start " << (int)isPlaying);
//
//	isPrerolling = false;
//	play();
//}
