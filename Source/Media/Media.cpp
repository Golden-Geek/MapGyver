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
	saveAndLoadRecursiveData = true;

	itemDataType = "Media";
	bitmapData = std::make_shared<Image::BitmapData>(image, Image::BitmapData::ReadWriteMode::writeOnly);

}

Media::~Media()
{
	OpenGLManager::getInstance()->openGLContext.executeOnGLThread([this](OpenGLContext& c) {
		c.makeActive();
		texture.release();
		}, true);
}

void Media::onContainerParameterChangedInternal(Parameter* p) {
}


void Media::updateVersion() {
	imageVersion = (imageVersion + 1) % 65535;
}

void Media::updateTexture()
{
	OpenGLManager::getInstance()->openGLContext.executeOnGLThread([this](OpenGLContext &c){ 
		c.makeActive();
		texture.loadImage(image);
	}, true);
}

Point<int> Media::getMediaSize()
{
	return Point<int>(image.getWidth(), image.getHeight());
}
