/*
  ==============================================================================

	MediaListItem.h
	Created: 18 Feb 2026 8:24:02am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ShaderMedia;

class MediaListItem :
	public BaseItem,
	public MediaTarget,
	public Media::AsyncListener
{
public:
	MediaListItem(const String& name = "Layer", var params = var());
	virtual ~MediaListItem();

	enum TransitionState { IDLE, LOADING, UNLOADING, RUNNING };

	Media* media;
	std::unique_ptr<ShaderMedia> shaderMedia; //if media is a shader media, keep a reference to it for easier access to shader parameters
	
	Trigger* launch;
	FloatParameter* transitionTime;
	FloatParameter* weight;
	EnumParameter* state;

	FloatParameter* transitionProgression;
	TargetParameter* transitionSourceMedia;
	TargetParameter* transitionTargetMedia;

	BoolParameter* autoPlay;
	BoolParameter* autoStop;

	enum AutoNextBehavior { AUTO_NEXT_OFF, AUTO_NEXT_MEDIA_FINISH, AUTO_NEXT_TIME };
	EnumParameter* autoNextBehavior;
	FloatParameter* autoNextTime;

	double timeAtStart = 0.f;
	float weightAtStart = 0.f;
	double targetTime = 0.f;
	float targetWeight = 0.f;
	bool forceRenderShader = false;

	void clearItem() override;

	void load(float fadeInTime, WeakReference<Media> prevMedia);
	void unload(float fadeOutTime);

	void process();

	virtual void onContainerParameterChangedInternal(Parameter* p) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	virtual void setMedia(Media* m);

	bool isUsingMedia(Media* m) override;
	bool isLoading() const;;
	bool isUnloading() const;;

	void newMessage(const Media::MediaEvent& event) override;

	DECLARE_ASYNC_EVENT(MediaListItem, MediaListItem, listItem, ENUM_LIST(AUTO_NEXT), EVENT_ITEM_CHECK);
};


class ReferenceMediaListItem : public MediaListItem
{
public:
	ReferenceMediaListItem(var params = var());
	virtual ~ReferenceMediaListItem();

	TargetParameter* targetMedia;

	void onContainerParameterChangedInternal(Parameter* p) override;

	DECLARE_TYPE("Reference List Item");
};

class OwnedMediaListItem : public MediaListItem
{
public:
	OwnedMediaListItem(var params = var());
	virtual ~OwnedMediaListItem();

	void setMedia(Media* m) override;

	std::unique_ptr<Media> ownedMedia;

	String getTypeString() const override { return ownedMedia != nullptr ? ownedMedia->getTypeString() : ""; }
	static OwnedMediaListItem* create(var params) { return new OwnedMediaListItem(params); }
};
