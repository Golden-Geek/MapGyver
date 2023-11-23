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
	Media(name, params),
	textureID(0)
{
}

ImageMedia::~ImageMedia()
{
}

void ImageMedia::renderGL()
{

	GenericScopedLock lock(imageLock);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.getWidth(), image.getHeight(), GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getWidth(), image.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);)
	//glBindTexture(GL_TEXTURE_2D, frameBuffer.getTextureID());


	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(0, 1); glVertex2f(0, image.getHeight());
	glTexCoord2f(1, 1); glVertex2f(image.getWidth(), image.getHeight());
	glTexCoord2f(1, 0); glVertex2f(image.getWidth(), 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFinish();
}

void ImageMedia::initFrameBuffer()
{
	frameBuffer.release();
	frameBuffer.initialise(GlContextHolder::getInstance()->context, image);

	if(textureID != 0) glDeleteTextures(1, &textureID);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getWidth(), image.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmapData->data);

	glBindTexture(GL_TEXTURE_2D, 0);

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
