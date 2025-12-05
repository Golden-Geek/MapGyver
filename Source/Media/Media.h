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
	BoolParameter* doNotPreview;

	IntParameter* width;
	IntParameter* height;

	enum StretchMode { STRETCH, FIT, FILL };
	EnumParameter* stretchMode;
	Trigger* generatePreview;

	ControllableContainer mediaParams;

	OpenGLFrameBuffer frameBuffer;
	bool alwaysRedraw;
	bool shouldRedraw;
	bool forceRedraw;
	bool autoClearFrameBufferOnRender;
	bool autoClearWhenNotUsed;

	HashMap<String, OpenGLFrameBuffer*> frameBufferMap; //for media that have multiple frame buffers (like multi-pass shaders)

	Array<MediaTarget*> usedTargets;

	bool manualRender;
	double timeAtLastRender;
	double customTime;

	FloatParameter* currentFPS;
	double lastFPSTick;
	double lastFPSHistory[10]{};
	int lastFPSIndex;
	bool customFPSTick;

	bool isEditing;

	Image previewImage;
	bool shouldGeneratePreviewImage;

	void clearItem() override;

	void onContainerTriggerTriggered(Trigger* t) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void newOpenGLContextCreated() override;
	virtual void renderOpenGL() override;
	void renderOpenGLMedia(bool force = false);
	void openGLContextClosing() override;

	virtual void initFrameBuffer();

	virtual void initGLInternal() {}
	virtual void preRenderGLInternal() {}
	virtual void renderGLInternal() {}
	virtual void closeGLInternal() {}



	void addFrameBuffer(const String& name, OpenGLFrameBuffer* f);
	void removeFrameBuffer(const String& name);
	String getNameForFrameBuffer(OpenGLFrameBuffer* f);
	StringArray getFrameBufferNames(); //for media that have multiple frame buffers (like multi-pass shaders)
	OpenGLFrameBuffer* getFrameBuffer(const String& name = String());
	GLint getTextureID(const String& name = String());
	void fillFrameBufferOptions(EnumParameter* e);

	bool hasSubFrameBuffers();

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
	
	void FPSTick();


	void setIsEditing(bool editing);
	virtual void updateBeingUsed();

	virtual Point<int> getMediaSize(const String& name = String()); //default to main fbo size
	virtual Point<int> getDefaultMediaSize(); //default size when no media is loaded
	
	Rectangle<int> getMediaRect(Rectangle<int> targetRect);
 
	virtual double getMediaLength() { return -1; } //for video and other time-based media
	virtual String getMediaContentName() const { return getTypeString(); }

	DECLARE_ASYNC_EVENT(Media, Media, media, ENUM_LIST(EDITING_CHANGED, PREVIEW_CHANGED, SUB_FRAMEBUFFERS_CHANGED, MEDIA_LENGTH_CHANGED, MEDIA_CONTENT_CHANGED), EVENT_ITEM_CHECK);


	JUCE_DECLARE_WEAK_REFERENCEABLE(Media);
};

class IInteractableMedia
{
public:


	virtual void sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect) = 0;
	virtual void sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect) = 0;
	virtual void sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect) = 0;
	virtual void sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect) = 0;
	virtual void sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) = 0;
	virtual void sendKeyPressed(const KeyPress& key) = 0;
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

	virtual Point<int> getDefaultMediaSize() override;

};