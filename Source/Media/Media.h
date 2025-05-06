/*
  ==============================================================================

	Object.h
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#pragma once



class Media :
	public BaseItem,
	public OpenGLRenderer
{
public:
	Media(const String& name = "Media", var params = var(), bool hasCustomSize = false);
	virtual ~Media();

	BoolParameter* isBeingUsed;

	IntParameter* width;
	IntParameter* height;
	Trigger* generatePreview;

	ControllableContainer mediaParams;

	OpenGLFrameBuffer frameBuffer;
	bool alwaysRedraw;
	bool shouldRedraw;
	bool forceRedraw;
	bool autoClearFrameBufferOnRender;
	bool autoClearWhenNotUsed;

	Array<MediaTarget*> usedTargets;

	bool manualRender;
	double timeAtLastRender;
	double customTime;

	FloatParameter* currentFPS;
	double lastFPSTick;
	double lastFPSHistory[10]{};
	int lastFPSIndex;
	bool customFPSTick;
	void FPSTick();

	bool isEditing;

	Image previewImage;
	bool shouldGeneratePreviewImage;
	
	void onContainerTriggerTriggered(Trigger* t) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void renderOpenGLMedia(bool force = false);
	void openGLContextClosing() override;

	virtual void initFrameBuffer();

	virtual void initGLInternal() {}
	virtual void preRenderGLInternal() {}
	virtual void renderGLInternal() {}
	virtual void closeGLInternal() {}

	OpenGLFrameBuffer* getFrameBuffer();
	GLint getTextureID();

	virtual void generatePreviewImage();

	void registerTarget(MediaTarget* target);
	void unregisterTarget(MediaTarget* target);

	//for sequence or other meta-media systems
	void setCustomTime(double time, bool seekMode = false);
	virtual void handleEnter(double time, bool play = false);
	virtual void handleExit();
	virtual void handleSeek(double time) {}
	virtual void handleStop() {}
	virtual void handleStart() {}


	void setIsEditing(bool editing);
	virtual void updateBeingUsed();

	virtual Point<int> getMediaSize();
	virtual double getMediaLength() { return -1; }

	DECLARE_ASYNC_EVENT(Media, Media, media, ENUM_LIST(EDITING_CHANGED, PREVIEW_CHANGED), EVENT_ITEM_CHECK);


	JUCE_DECLARE_WEAK_REFERENCEABLE(Media);
};


class ImageMedia :
	public Media
{
public:
	ImageMedia(const String& name = "CPUMedia", var params = var());
	virtual ~ImageMedia();

	CriticalSection imageLock;
	Image image;
	std::shared_ptr<Image::BitmapData> bitmapData;
	juce::OpenGLFrameBuffer imageFBO;

	virtual void preRenderGLInternal() override;
	virtual void renderGLInternal();
	virtual void initFrameBuffer() override;

	void initImage(int width, int height);
	virtual void initImage(const Image& image);

	virtual Point<int> getMediaSize() override;

};