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
	public MediaListSubItem::AsyncListener
{
public:
	MediaListItem(const String& name = "Layer", var params = var());
	virtual ~MediaListItem();

	enum TransitionState { IDLE, LOADING, UNLOADING, RUNNING };
	
	Trigger* launch;
	FloatParameter* transitionTime;
	FloatParameter* weight;
	EnumParameter* state;

	BoolParameter* autoPlay;
	BoolParameter* autoStop;

	enum AutoNextBehavior { AUTO_NEXT_OFF, AUTO_NEXT_MEDIA_FINISH_FIRST, AUTO_NEXT_MEDIA_FINISH_LAST, AUTO_NEXT_TIME };
	EnumParameter* autoNextBehavior;
	FloatParameter* autoNextTime;

	double timeAtStart = 0.f;
	float weightAtStart = 0.f;
	double targetTime = 0.f;
	float targetWeight = 0.f;

	OwnedArray<MediaListSubItem> subItems;

	void clearItem() override;

	void load(float fadeInTime, MediaListItem* prevItem);
	void unload(float fadeOutTime);

	void render();
	void process();

	Media* getMediaAt(int index);
	OpenGLFrameBuffer* getFrameBufferAt(int index);
	GLuint getTextureIDAt(int index);
	ShaderMedia* getShaderMediaAt(int index);

	float getReferenceLength();

	virtual void onContainerParameterChangedInternal(Parameter* p) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	bool isLoading() const;
	bool isUnloading() const;

	void setNumLayers(int num);

	void newMessage(const MediaListSubItem::MediaListSubItemEvent& event) override;

	var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataItemInternal(var data) override;

	DECLARE_ASYNC_EVENT(MediaListItem, MediaListItem, listItem, ENUM_LIST(AUTO_NEXT, SELECTION_CHANGED), EVENT_ITEM_CHECK);
};