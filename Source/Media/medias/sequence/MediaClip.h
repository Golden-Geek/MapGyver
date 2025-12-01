/*
  ==============================================================================

	MediaClip.h
	Created: 21 Dec 2023 10:40:39am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#define CLIP_MEDIA_ID 0

class ClipTransition;

class MediaClip :
	public LayerBlock,
	public MediaTarget,
	public Media::AsyncListener
{
public:
	MediaClip(const String& name, var params = var());
	virtual ~MediaClip();

	void clearItem() override;

	double relativeTime;
	bool isPlaying;
	bool activationChanged;

	Media* media;
	WeakReference<Inspectable> mediaRef;

	FloatParameter* fadeIn;
	FloatParameter* fadeOut;
	Automation fadeCurve;
	bool settingLengthFromMethod;
	
	FloatParameter* preStart;
	FloatParameter* postEnd;

	ClipTransition* inTransition;
	ClipTransition* outTransition;

	virtual void setMedia(Media* m);

	virtual void setTime(double time, bool seekMode);
	void setIsPlaying(bool playing);

	void setInTransition(ClipTransition* t);
	void setOutTransition(ClipTransition* t);

	void dispatchTransitionChanged();

	void onContainerParameterChangedInternal(Parameter* p) override;
	virtual void controllableStateChanged(Controllable* c) override;

	void setCoreLength(float value, bool stretch = false, bool stickToCoreEnd = false) override;

	float getFadeMultiplier();

	void updateCoreRangeFromMedia();

	bool isUsingMedia(Media* m) override;

	void newMessage(const Media::MediaEvent& e) override;

	class  MediaClipListener
	{
	public:
		/** Destructor. */
		virtual ~MediaClipListener() {}
		virtual void mediaChanged(MediaClip*) {}
		virtual void mediaClipFadesChanged(MediaClip*) {}
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(MediaClip, mediaClip);

	DECLARE_ASYNC_EVENT(MediaClip, MediaClip, mediaClip, ENUM_LIST(MEDIA_CHANGED, FADES_CHANGED, TRANSITIONS_CHANGED, PREVIEW_CHANGED), EVENT_ITEM_CHECK);

	DECLARE_TYPE("Media Clip");
};

class ReferenceMediaClip :
	public MediaClip
{
public:
	ReferenceMediaClip(var params = var());
	virtual ~ReferenceMediaClip();

	TargetParameter* mediaTarget;
	void onContainerParameterChangedInternal(Parameter* p) override;

	DECLARE_TYPE("Reference Clip");
};

class OwnedMediaClip :
	public MediaClip
{
public:
	OwnedMediaClip(var params = var());
	OwnedMediaClip(Media* m);
	virtual ~OwnedMediaClip();

	std::unique_ptr<Media> ownedMedia;


	void setMedia(Media* m) override;

	String getTypeString() const override { return media != nullptr ? media->getTypeString() : ""; }
	static OwnedMediaClip* create(var params = var()) { return new OwnedMediaClip(params); }
};