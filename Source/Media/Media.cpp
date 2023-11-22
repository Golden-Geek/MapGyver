/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "Media.h"

MediaUI::MediaUI(Media* item) :
	BaseItemUI(item)
{
}

MediaUI::~MediaUI()
{
}


Media::Media(const String& name, var params) :
	BaseItem(name),
	imageVersion(0),
	image(Image::ARGB, 10, 10, true)
{
	GlContextHolder::getInstance()->registerOpenGlRenderer(this);
	saveAndLoadRecursiveData = true;

	itemDataType = "Media";
	bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::ReadWriteMode::readWrite);

}

Media::~Media()
{
}

void Media::onContainerParameterChangedInternal(Parameter* p) {
}

void Media::newOpenGLContextCreated()
{
}

void Media::renderOpenGL()
{
}

void Media::openGLContextClosing()
{
}

Point<int> Media::getMediaSize()
{
	return Point<int>(image.getWidth(), image.getHeight());
}
