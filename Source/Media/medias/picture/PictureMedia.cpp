/*
  ==============================================================================

	PictureMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

PictureMedia::PictureMedia(var params) :
	ImageMedia(getTypeString(), params)
{
	filePath = addFileParameter("File path", "File path", "");
}

PictureMedia::~PictureMedia()
{
}

void PictureMedia::onContainerParameterChanged(Parameter* p)
{
	Media::onContainerParameterChanged(p);

	if (p == filePath)
	{
		File target = filePath->getFile();

		if (target.existsAsFile() && target.hasFileExtension("jpg;jpeg;png"))
		{
			GenericScopedLock lock(imageLock);
			initImage(ImageFileFormat::loadFrom(target));
			shouldUpdateImage = true;
		}
	}
}