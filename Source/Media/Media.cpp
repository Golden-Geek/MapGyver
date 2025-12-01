/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Engine/MGEngine.h"

using namespace juce::gl;

Media::Media(const String& name, var params, bool hasCustomSize) :
	BaseItem(name),
	width(nullptr),
	height(nullptr),
	customTime(-1),
	mediaParams("Media Parameters"),
	alwaysRedraw(true),
	shouldRedraw(false),
	forceRedraw(false),
	autoClearFrameBufferOnRender(true),
	autoClearWhenNotUsed(true),
	timeAtLastRender(0),
	lastFPSTick(0),
	lastFPSIndex(0),
	customFPSTick(false),
	isEditing(false),
	shouldGeneratePreviewImage(true),
	mediaNotifier(5)
{
	setHasCustomColor(true);
	saveAndLoadRecursiveData = true;

	addChildControllableContainer(&mediaParams);

	isBeingUsed = addBoolParameter("isBeingUsed", "Is being used", false);
	isBeingUsed->setControllableFeedbackOnly(true);

	doNotPreview = addBoolParameter("Do Not Preview", "If enabled, no media preview will not automatically show this when selected", false);

	if (hasCustomSize)
	{
		width = addIntParameter("Width", "Width of the media", 1920, 1, 10000);
		height = addIntParameter("Height", "Height of the media", 1080, 1, 10000);
	}

	currentFPS = addFloatParameter("current FPS", "", 0, 0, 60);
	currentFPS->isSavable = false;
	currentFPS->enabled = false;
	generatePreview = addTrigger("Generate Preview", "Generate a preview image of the media");

	manualRender = params.getProperty("manualRender", false);
	if (!manualRender) GlContextHolder::getInstance()->registerOpenGlRenderer(this, 1);


	itemDataType = "Media";
}

Media::~Media()
{
	if (!manualRender && GlContextHolder::getInstanceWithoutCreating() != nullptr) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
}


void Media::onContainerTriggerTriggered(Trigger* t)
{
	if (t == generatePreview)
	{
		shouldGeneratePreviewImage = true;
	}
}

void Media::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (cc == &mediaParams)
	{
		shouldRedraw = true;
	}
}

void Media::newOpenGLContextCreated()
{
	initGLInternal();
}

void Media::renderOpenGL()
{
	renderOpenGLMedia();
}

void Media::renderOpenGLMedia(bool force)
{
	if (isClearing) return;

	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (frameBuffer.getWidth() != size.x || frameBuffer.getHeight() != size.y) initFrameBuffer();

	if (forceRedraw)
	{
		force = true;
	}

	bool shouldRenderContent = (enabled->boolValue() && isBeingUsed->boolValue()) || forceRedraw || shouldGeneratePreviewImage;

	forceRedraw = false;

	if (!shouldRenderContent && !force) return;


	const double frameTime = 1000.0 / RMPSettings::getInstance()->fpsLimit->intValue();
	double t = GlContextHolder::getInstance()->timeAtRender;
	if (t < timeAtLastRender + frameTime && !force) return;

	//log delta
	//LOG("Delta: " << t - timeAtLastRender);
	timeAtLastRender = t;

	if (!frameBuffer.isValid()) return;

	if (shouldRedraw || shouldGeneratePreviewImage || alwaysRedraw || force)
	{
		if (dynamic_cast<SequenceMedia*>(this) == nullptr)
		{
			//NLOG(niceName, "Prerender GL Media");
		}

		preRenderGLInternal(); //allow for pre-rendering operations even if not being used or disabled

		if (autoClearFrameBufferOnRender)
		{
			frameBuffer.makeCurrentAndClear();
		}
		else
		{
			frameBuffer.makeCurrentRenderingTarget();
		}

		Init2DViewport(frameBuffer.getWidth(), frameBuffer.getHeight());
		glColor4f(0, 0, 0, 1);

		if (shouldRenderContent)
		{
			renderGLInternal();
		}

		if (shouldGeneratePreviewImage)
		{
			generatePreviewImage();
			shouldGeneratePreviewImage = false;
		}
		frameBuffer.releaseAsRenderingTarget();
		shouldRedraw = false;

		if (!customFPSTick) FPSTick();
	}
}



void Media::addFrameBuffer(const String& name, OpenGLFrameBuffer* f)
{
	jassert(f != &frameBuffer);
	if (frameBufferMap.contains(name) && frameBufferMap[name] == f) return;

	frameBufferMap.set(name, f);
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::SUB_FRAMEBUFFERS_CHANGED, this));
}

void Media::removeFrameBuffer(const String& name)
{
	if (!frameBufferMap.contains(name)) return;
	if (Engine::mainEngine->isClearing) return;

	frameBufferMap.remove(name);
	mediaNotifier.addMessage(new MediaEvent(MediaEvent::SUB_FRAMEBUFFERS_CHANGED, this));
}

String Media::getNameForFrameBuffer(OpenGLFrameBuffer* f)
{
	HashMap<String, OpenGLFrameBuffer*>::Iterator it(frameBufferMap);
	while (it.next())
	{
		if (it.getValue() == f) return it.getKey();
	}

	return "";
}

StringArray Media::getFrameBufferNames() {
	StringArray result("Default", "");
	HashMap<String, OpenGLFrameBuffer*>::Iterator it(frameBufferMap);
	while (it.next()) result.add(it.getKey());
	return result;
}


OpenGLFrameBuffer* Media::getFrameBuffer(const String& name)
{
	if (name.isEmpty()) return &frameBuffer;

	if (frameBufferMap.contains(name)) return frameBufferMap[name];
	return nullptr;
}

GLint Media::getTextureID(const String& name)
{
	if (name.isEmpty()) return frameBuffer.getTextureID();

	if (auto* fb = getFrameBuffer(name)) return fb->getTextureID();
	return 0;
}

void Media::fillFrameBufferOptions(EnumParameter* e)
{
	Array<EnumParameter::EnumValue> options;
	options.add({ "Default", "" });
	HashMap<String, OpenGLFrameBuffer*>::Iterator it(frameBufferMap);
	while (it.next())
	{
		options.add({ it.getKey(), it.getKey() });
	}
	e->setOptions(options);
}

bool Media::hasSubFrameBuffers() {
	return frameBufferMap.size() > 0;
}

void Media::generatePreviewImage()
{
	if (frameBuffer.isValid())
	{
		const int width = frameBuffer.getWidth();
		const int height = frameBuffer.getHeight();
		Image img = Image(Image::ARGB, width, height, true);

		{
			Image::BitmapData bitmapData(img, Image::BitmapData::writeOnly);
			frameBuffer.readPixels(reinterpret_cast<juce::PixelARGB*> (bitmapData.data), Rectangle<int>(0, 0, width, height), OpenGLFrameBuffer::RowOrder::fromTopDown);
		}
		if (width > 200 || height > 200)
		{
			float scale = jmin(200.0f / width, 200.0f / height);
			previewImage = img.rescaled(width * scale, height * scale);
		}

		mediaNotifier.addMessage(new MediaEvent(MediaEvent::PREVIEW_CHANGED, this));
	}


}

void Media::registerTarget(MediaTarget* target)
{
	usedTargets.addIfNotAlreadyThere(target);
	updateBeingUsed();
}

void Media::unregisterTarget(MediaTarget* target)
{
	usedTargets.removeAllInstancesOf(target);
	updateBeingUsed();
}

//for sequence or other meta-media systems
void Media::setCustomTime(double time, bool seekMode)
{
	customTime = time;
	shouldRedraw = true;
	if (customTime >= 0 && seekMode) handleSeek(customTime);
}


void Media::handleEnter(double time, bool play)
{
	setCustomTime(time, !play);
}

void Media::handleExit()
{
	setCustomTime(-1);
}

void Media::updateBeingUsed()
{
	if (isClearing || Engine::mainEngine->isClearing) return;

	if (usedTargets.size() > 0)
	{
		for (auto& u : usedTargets)
		{
			if (u->isUsingMedia(this))
			{
				isBeingUsed->setValue(true);
				return;
			}
		}
	}

	isBeingUsed->setValue(false);
}

void Media::setIsEditing(bool editing)
{
	if (isEditing == editing) return;
	isEditing = editing;
	if (!isClearing)	mediaNotifier.addMessage(new MediaEvent(MediaEvent::EDITING_CHANGED, this));
}

void Media::openGLContextClosing()
{
	closeGLInternal();
	frameBuffer.release();
}

void Media::initFrameBuffer()
{
	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (frameBuffer.isValid()) frameBuffer.release();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, size.x, size.y);
	shouldRedraw = true;
}

Point<int> Media::getMediaSize(const String& name)
{
	if (name.isNotEmpty()) {
		if (auto* fb = getFrameBuffer(name)) {
			if (fb->isValid()) return Point<int>(fb->getWidth(), fb->getHeight());
		}
	}

	return getDefaultMediaSize();
}


Point<int> Media::getDefaultMediaSize()
{
	if (width != nullptr && height != nullptr) return Point<int>(width->intValue(), height->intValue());
	if (frameBuffer.isValid()) return Point<int>(frameBuffer.getWidth(), frameBuffer.getHeight());
	return Point<int>(0, 0);
}



void Media::FPSTick()
{
	double currentTime = juce::Time::getMillisecondCounterHiRes();
	double elapsedMillis = currentTime - lastFPSTick;
	lastFPSTick = currentTime;
	lastFPSIndex = (lastFPSIndex + 1) % 10;
	lastFPSHistory[lastFPSIndex] = elapsedMillis;

	if (lastFPSIndex != 0) return;

	double averageElapsed = 0;
	for (int i = 0; i < 10; i++) {
		averageElapsed += lastFPSHistory[i];
	}
	averageElapsed /= 10;

	float fps = 1000.0 / averageElapsed;
	int max = ceil(fps / 10.0) * 10;
	if ((float)currentFPS->maximumValue < max) {
		currentFPS->maximumValue = max;
	}
	currentFPS->setValue(fps);

	//if (isClearing) return;
	// Calcul des FPS
	//MessageManager::callAsync([this, max, fps]()
	//	{
	//		if (Engine::mainEngine == nullptr || Engine::mainEngine->isClearing) return;
	//		if (isClearing) return;

	//		if ((float)currentFPS->maximumValue < max) {
	//			currentFPS->maximumValue = max;
	//		}
	//		currentFPS->setValue(fps);
	//	});
}



// ImageMedia

ImageMedia::ImageMedia(const String& name, var params) :
	Media(name, params)
{
}

ImageMedia::~ImageMedia()
{
}

void ImageMedia::preRenderGLInternal()
{
	GenericScopedLock lock(imageLock);
	imageFBO.makeCurrentAndClear();
	glBindTexture(GL_TEXTURE_2D, imageFBO.getTextureID());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.getWidth(), image.getHeight(), GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);
	imageFBO.releaseAsRenderingTarget();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageMedia::renderGLInternal()
{
	glColor4f(1, 1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, imageFBO.getTextureID());
	Draw2DTexRectFlipped(0, 0, imageFBO.getWidth(), imageFBO.getHeight());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageMedia::initFrameBuffer()
{
	GenericScopedLock lock(imageLock);
	Media::initFrameBuffer();
	if (imageFBO.isValid()) imageFBO.release();
	imageFBO.initialise(GlContextHolder::getInstance()->context, frameBuffer.getWidth(), frameBuffer.getHeight());

}

void ImageMedia::initImage(int width, int height)
{
	initImage(Image(Image::ARGB, width, height, true));
}

void ImageMedia::initImage(const Image& newImage)
{
	GenericScopedLock lock(imageLock);

	shouldRedraw = true;

	if (!newImage.isValid())
	{
		image = Image();
		return;
	}

	bool imageUpdated = false;
	if (newImage.getWidth() != image.getWidth() || newImage.getHeight() != image.getHeight())
	{
		image = newImage.convertedToFormat(Image::ARGB);

		imageUpdated = true;
		bitmapData.reset();

	}

	if (imageUpdated && newImage.isValid())
	{
		//Graphics graphics(image);
		//graphics.drawImage(newImage, Rectangle<float>(0, 0, image.getWidth(), image.getHeight()));
	}

	if (bitmapData == nullptr)
	{
		bitmapData.reset(new Image::BitmapData(image, Image::BitmapData::readWrite));
	}
}

Point<int> ImageMedia::getDefaultMediaSize()
{
	return Point<int>(image.getWidth(), image.getHeight());
}
