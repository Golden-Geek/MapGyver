/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Media.h"

Media::Media(const String& name, var params, bool hasCustomSize) :
	BaseItem(name),
	width(nullptr),
	height(nullptr),
	alwaysRedraw(false),
	shouldRedraw(false),
	flipY(false)
{
	if (hasCustomSize)
	{
		width = addIntParameter("Width", "Width of the media", 1920, 1, 10000);
		height = addIntParameter("Height", "Height of the media", 1080, 1, 10000);
	}

	GlContextHolder::getInstance()->registerOpenGlRenderer(this);
	saveAndLoadRecursiveData = true;

	itemDataType = "Media";
}

Media::~Media()
{
	if (GlContextHolder::getInstanceWithoutCreating() != nullptr) GlContextHolder::getInstance()->unregisterOpenGlRenderer(this);
}

void Media::onContainerParameterChangedInternal(Parameter* p) {
}

void Media::newOpenGLContextCreated()
{
	initGLInternal();
}

void Media::renderOpenGL()
{
	if (!enabled->boolValue()) return;
	if(!isBeingUsed()) return;

	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (frameBuffer.getWidth() != size.x || frameBuffer.getHeight() != size.y) initFrameBuffer();

	if (!frameBuffer.isValid()) return;

	if (shouldRedraw || alwaysRedraw)
	{
		frameBuffer.makeCurrentRenderingTarget();
		renderGLInternal();
		frameBuffer.releaseAsRenderingTarget();
		shouldRedraw = false;
	}
}

OpenGLFrameBuffer* Media::getFrameBuffer()
{
	return &frameBuffer;
}

GLint Media::getTextureID()
{
	return getFrameBuffer()->getTextureID();
}

void Media::registerTarget(MediaTarget* target)
{
	usedTargets.addIfNotAlreadyThere(target);
}

void Media::unregisterTarget(MediaTarget* target)
{
	usedTargets.removeAllInstancesOf(target);
}

bool Media::isBeingUsed()
{
	if(usedTargets.size() == 0) return false;
	for(auto & u : usedTargets) if(u->isUsingMedia(this)) return true;
	return false;
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

	frameBuffer.initialise(GlContextHolder::getInstance()->context, size.x, size.y);
	shouldRedraw = true;
}

Point<int> Media::getMediaSize()
{
	if(width != nullptr && height != nullptr) return Point<int>(width->intValue(), height->intValue());
	return Point<int>(0, 0);
}


// ImageMedia

ImageMedia::ImageMedia(const String& name, var params) :
	Media(name, params)
{
	flipY = true;
}

ImageMedia::~ImageMedia()
{
}

void ImageMedia::renderGLInternal()
{
	GenericScopedLock lock(imageLock);

	glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.getWidth(), image.getHeight(), GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageMedia::initFrameBuffer()
{
	GenericScopedLock lock(imageLock);
	if (frameBuffer.isValid()) frameBuffer.release();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, image);
	shouldRedraw = true;
}

void ImageMedia::initImage(int width, int height)
{
	initImage(Image(Image::ARGB, width, height, true));
}

void ImageMedia::initImage(Image newImage)
{
	if (!newImage.isValid())
	{
		image = Image();
		shouldRedraw = true;
		return;
	}

	image = newImage.convertedToFormat(Image::ARGB);
	bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::readWrite);
	shouldRedraw = true;
}

Point<int> ImageMedia::getMediaSize()
{
	return Point<int>(image.getWidth(), image.getHeight());
}
