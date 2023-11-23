/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

Media::Media(const String& name, var params) :
	BaseItem(name),
	shouldRedraw(false)
{
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
	initGL();
}

void Media::renderOpenGL()
{
	Point<int> size = getMediaSize();
	if (size.isOrigin()) return;
	if (frameBuffer.getWidth() != size.x || frameBuffer.getHeight() != size.y) initFrameBuffer();

	if (!frameBuffer.isValid()) return;

	if (shouldRedraw)
	{
		frameBuffer.makeCurrentRenderingTarget();
		renderGL();
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

void Media::openGLContextClosing()
{
	closeGL();
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
	return Point<int>(0, 0);
}


// ImageMedia

ImageMedia::ImageMedia(const String& name, var params) :
	Media(name, params)
{
}

ImageMedia::~ImageMedia()
{
}

void ImageMedia::renderGL()
{
	GenericScopedLock lock(imageLock);

	glMatrixMode(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.getWidth(), image.getHeight(), GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageMedia::initFrameBuffer()
{
	frameBuffer.initialise(GlContextHolder::getInstance()->context, image);
	shouldRedraw = true;
}

void ImageMedia::initImage(int width, int height)
{
	initImage(Image(Image::ARGB, width, height, true));
}

void ImageMedia::initImage(Image newImage)
{
	image = newImage.convertedToFormat(Image::ARGB);
	bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::readWrite);
	shouldRedraw = true;
}

Point<int> ImageMedia::getMediaSize()
{
	return Point<int>(image.getWidth(), image.getHeight());
}
