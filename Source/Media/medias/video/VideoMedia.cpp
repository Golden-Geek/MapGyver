/*
  ==============================================================================

	VideoMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/RMPEngine.h"

VideoMedia::VideoMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	source = addEnumParameter("Source", "Source");
	source->addOption("File", Source_File)->addOption("URL", Source_URL);

	filePath = addFileParameter("File path", "File path", "");
	url = addStringParameter("URL", "URL", "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", false);

	playAtLoad = addBoolParameter("Play at load", "Play at load", false);

	startBtn = addTrigger("start", "");
	stopBtn = addTrigger("stop", "");
	restartBtn = addTrigger("restart", "");
	pauseBtn = addTrigger("pause", "");

	mediaVolume = addFloatParameter("Volume", "Media volume", 1, 0, 1);
	seek = addFloatParameter("Seek", "Manual seek", 1, 0, 1);
	seek->isSavable = false;

	speedRate = addFloatParameter("Speed rate", "Speed factor of video", 1, 0);
	beatPerCycle = addIntParameter("Beat by cycles", "Number of tap tempo beats by cycle", 1, 1);
	tapTempoBtn = addTrigger("Tap tempo", "");

	RMPEngine* e = dynamic_cast<RMPEngine*>(Engine::mainEngine);
	VLCInstance = e->VLCInstance;
	frameUpdated = false;
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
		stop();
		VLCMediaList = libvlc_media_list_new(VLCInstance);
		
		VideoSource s = source->getValueDataAsEnum<VideoSource>();
		
		if(s == Source_File) VLCMedia = libvlc_media_new_path(VLCInstance, filePath->getFile().getFullPathName().toRawUTF8());
		else VLCMedia = libvlc_media_new_location(VLCInstance, url->stringValue().toRawUTF8());

		libvlc_media_list_add_media(VLCMediaList, VLCMedia);

		VLCMediaPlayer = libvlc_media_player_new(VLCInstance); //libvlc_media_player_new_from_media(VLCMedia);
		libvlc_video_set_format_callbacks(VLCMediaPlayer, setup_video, cleanup_video);
		libvlc_video_set_callbacks(VLCMediaPlayer, lock, unlock, display, this);

		VLCMediaListPlayer = libvlc_media_list_player_new(VLCInstance);

		libvlc_media_list_player_set_media_list(VLCMediaListPlayer, VLCMediaList);
		libvlc_media_list_player_set_media_player(VLCMediaListPlayer, VLCMediaPlayer);
		libvlc_media_list_player_set_playback_mode(VLCMediaListPlayer, libvlc_playback_mode_loop);

		libvlc_event_attach(libvlc_media_player_event_manager(VLCMediaPlayer), libvlc_MediaPlayerPositionChanged, vlcSeek, this);
		libvlc_media_player_play(VLCMediaPlayer);

		libvlc_media_release(VLCMedia); VLCMedia = nullptr;
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
		if (!vlcSeekedLast)
		{
			libvlc_media_player_set_position(VLCMediaPlayer, seek->floatValue());
		}
		vlcSeekedLast = false;
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
	if (playAtLoad->boolValue()) play();
}

void VideoMedia::play()
{
	if (VLCMediaPlayer != nullptr) {
		libvlc_media_list_player_play(VLCMediaListPlayer);
	}
}

void VideoMedia::stop()
{
	if (VLCMediaPlayer != nullptr) {
		libvlc_media_list_player_stop(VLCMediaListPlayer);
	}
}

void VideoMedia::pause()
{
	libvlc_media_list_player_pause(VLCMediaListPlayer);

}

void VideoMedia::restart()
{
	stop();
	play();
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
	shouldRedraw = true;
	imageLock.exit();
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

	GenericScopedLock lock(imageLock);

	initImage(imageWidth, imageHeight);

	vlcDataIsValid = true;
	memcpy(chroma, "RV32", 4);
	(*pitches) = imageWidth * 4;
	(*lines) = imageHeight;

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
	seek->setValue(pos);
}

void VideoMedia::tapTempo()
{
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTapTempo;
	lastTapTempo = now;
	if (delta < 3000) {
		delta = delta * (int)beatPerCycle->getValue();
		float totalTime = libvlc_media_player_get_length(VLCMediaPlayer);
		float rate = totalTime / delta;
		speedRate->setValue(rate);
		//speed->setValue(cpm);
	}
}
