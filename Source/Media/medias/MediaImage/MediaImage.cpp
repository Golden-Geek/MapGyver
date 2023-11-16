/*
  ==============================================================================

	MediaImage.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaImage::MediaImage(var params) :
	Media(getTypeString(), params)
{
	filePath = addFileParameter("File path", "File path", "");
}

MediaImage::~MediaImage()
{
}

void MediaImage::clearItem()
{
	BaseItem::clearItem();
}

void MediaImage::onContainerParameterChanged(Parameter* p)
{
	Media::onContainerParameterChanged(p);
	if (p == filePath) {
		File target = filePath->getFile();
		if (target.existsAsFile() && target.hasFileExtension("jpg;jpeg;png")) {
			String ext = target.getFileExtension();
			image = ImageFileFormat::loadFrom(target);
			updateVersion();
		}
	}
}

