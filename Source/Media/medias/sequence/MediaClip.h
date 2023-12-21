/*
  ==============================================================================

    MediaClip.h
    Created: 21 Dec 2023 10:40:39am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#define CLIP_MEDIA_ID 0

class MediaClip :
	public LayerBlock,
	public MediaTarget
{
public:
	MediaClip(var params = var());
	~MediaClip();

	void clearItem() override;

	TargetParameter* mediaTarget;
	Media* media;

	FloatParameter* fadeIn;
	FloatParameter* fadeOut;
	Automation fadeCurve;
	bool settingLengthFromMethod;

	void setMedia(Media* m);

	void onContainerParameterChangedInternal(Parameter* p) override;
	virtual void controllableStateChanged(Controllable* c) override;

	void setCoreLength(float value, bool stretch = false, bool stickToCoreEnd = false) override;

	float getFadeMultiplier(float absoluteTime);

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
		enum Type { FADES_CHANGED, REGENERATE_PREVIEW };

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