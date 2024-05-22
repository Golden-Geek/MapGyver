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
	public MediaTarget
{
public:
	MediaClip(const String& name, var params = var());
	virtual ~MediaClip();

	void clearItem() override;

	double relativeTime;
	bool isPlaying;
	bool justActivated;

	Media* media;
	WeakReference<Inspectable> mediaRef;

	FloatParameter* fadeIn;
	FloatParameter* fadeOut;
	Automation fadeCurve;
	bool settingLengthFromMethod;
	
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

	bool isUsingMedia(Media* m) override;


	class  MediaClipListener
	{
	public:
		/** Destructor. */
		virtual ~MediaClipListener() {}
		virtual void mediaClipFadesChanged(MediaClip*) {}
	};

	ListenerList<MediaClipListener> mediaClipListeners;
	void addMediaClipListener(MediaClipListener* newListener) { mediaClipListeners.add(newListener); }
	void removeMediaClipListener(MediaClipListener* listener) { mediaClipListeners.remove(listener); }

	// ASYNC
	class  MediaClipEvent
	{
	public:
		enum Type { FADES_CHANGED, REGENERATE_PREVIEW, TRANSITIONS_CHANGED };

		MediaClipEvent(Type t, MediaClip* p, var v = var()) :
			type(t), mediaClip(p), value(v) {}

		Type type;
		MediaClip* mediaClip;
		var value;
	};

	QueuedNotifier<MediaClipEvent> mediaClipNotifier;
	typedef QueuedNotifier<MediaClipEvent>::Listener AsyncListener;

	void addAsyncMediaClipListener(AsyncListener* newListener) { mediaClipNotifier.addListener(newListener); }
	void addAsyncCoalescedMediaClipListener(AsyncListener* newListener) { mediaClipNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncMediaClipListener(AsyncListener* listener) { mediaClipNotifier.removeListener(listener); }

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

	DECLARE_TYPE("Reference");
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