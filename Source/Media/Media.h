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

	IntParameter* width;
	IntParameter* height;

	ControllableContainer mediaParams;

	OpenGLFrameBuffer frameBuffer;
	bool alwaysRedraw;
	bool shouldRedraw;
	bool flipY;

	Array<MediaTarget*> usedTargets;

	double timeAtLastRender;
	double customTime;

	FloatParameter* currentFPS;
	double lastFPSTick;
	double lastFPSHistory[10];
	int lastFPSIndex;
	bool customFPSTick;
	void FPSTick();

	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	virtual void initFrameBuffer();

	virtual void initGLInternal() {}
	virtual void preRenderGLInternal() {}
	virtual void renderGLInternal() {}
	virtual void closeGLInternal() {}

	OpenGLFrameBuffer* getFrameBuffer();
	GLint getTextureID();

	void registerTarget(MediaTarget* target);
	void unregisterTarget(MediaTarget* target);

	//for sequence or other meta-media systems
	void setCustomTime(double time, bool seekMode = false);
	virtual void handleEnter(double time);
	virtual void handleExit();
	virtual void handleSeek(double time) {}
	virtual void handleStop() {}
	virtual void handleStart() {}

	bool isBeingUsed();

	virtual Point<int> getMediaSize();
};


class ImageMedia :
	public Media
{
public:
	ImageMedia(const String& name = "CPUMedia", var params = var());
	virtual ~ImageMedia();

	SpinLock imageLock;
	Image image;
	std::shared_ptr<Image::BitmapData> bitmapData;
	std::shared_ptr<Graphics> graphics;

	virtual void renderGLInternal();
	virtual void initFrameBuffer() override;

	void initImage(int width, int height);
	virtual void initImage(const Image& image);

	virtual Point<int> getMediaSize() override;
};