/*
  ==============================================================================

	MediaListSubItem.h
	Created: 28 Feb 2026 9:58:37am
	Author:  bkupe

  ==============================================================================
*/

#pragma once
class ShaderMedia;

class MediaListSubItem :
	public ControllableContainer,
	public MediaTarget,
	public Media::AsyncListener,
	public Inspectable::AsyncListener

{
public:
	MediaListSubItem(const String& name = "Layer", bool canBeSubTexture = true);
	virtual ~MediaListSubItem();

	enum TransitionState { IDLE, LOADING, UNLOADING, RUNNING };

	EnumParameter* type;

	std::unique_ptr<Media> ownedMedia;
	Media* media;
	EnumParameter* textureName;
	std::unique_ptr<ShaderMedia> shaderMedia; //if media is a shader media, keep a reference to it for easier access to shader parameters

	TargetParameter* reference;

	FloatParameter* transitionTimeOverride;

	FloatParameter* transitionProgression;
	TargetParameter* transitionSourceMedia;
	TargetParameter* transitionTargetMedia;

	bool forceRenderShader;

	CriticalSection mediaLock;

	float targetEndTransitionTime;
	float weightAtStart;
	float weight;

	void clear() override;

	void onContainerParameterChanged(Parameter* p) override;
	void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	void updateCurrentMedia(bool force = false);
	void setMedia(Media* m);

	void setupTransition(Media* source);
	void renderShaderIfNecessary();

	void updateTextureNameOptions(Media* forceMedia = nullptr);

	void render(bool isLoading);

	void newMessage(const Media::MediaEvent& event) override;
	void newMessage(const Inspectable::InspectableEvent& event) override;

	bool isSubTexture() const { return type->getValueData().toString() == "sub_texture"; }
	bool isReference() const { return type->getValueData().toString() == "reference"; }

	OpenGLFrameBuffer* getFrameBuffer(const String& forceTexName = "");
	GLuint getTextureID(const String& forceTexName = "");

	var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataInternal(var data) override;

	DECLARE_ASYNC_EVENT(MediaListSubItem, MediaListSubItem, listSubItem, ENUM_LIST(SUBMEDIA_FINISHED, SELECTION_CHANGED), EVENT_ITEM_CHECK);
};
