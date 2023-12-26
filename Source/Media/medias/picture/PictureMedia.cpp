/*
  ==============================================================================

	PictureMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

PictureMedia::PictureMedia(var params) :
	ImageMedia(getTypeString(), params),
	Thread("Picture Media")
{
	source = addEnumParameter("Source", "Source");
	source->addOption("File", Source_File)->addOption("URL", Source_URL);

	filePath = addFileParameter("File path", "File path", "");
	url = addStringParameter("URL", "URL", "https://i.pinimg.com/564x/9a/92/62/9a926291240989c77bc77d9d2d3fcec6.jpg", false);
	convertToLocal = addTrigger("Convert to local", "If online picture, downloads it aside the project file and points to it");
}

PictureMedia::~PictureMedia()
{
	stopThread(1000);
}

void PictureMedia::onContainerTriggerTriggered(Trigger* t)
{
	Media::onContainerTriggerTriggered(t);
	if (t == convertToLocal)
	{
		if (source->getValueDataAsEnum<PictureSource>() == Source_File) return;

		if (image.isValid())
		{
			PNGImageFormat png;
			File f = Engine::mainEngine->getFile().getParentDirectory().getChildFile(niceName + ".png");
			FileOutputStream fs(f);
			png.writeImageToStream(image, fs);
			LOG("File saved  to " << f.getFullPathName());

			filePath->setValue(f.getFullPathName());
			source->setValueWithData(Source_File);
		}
	}
}

void PictureMedia::onContainerParameterChanged(Parameter* p)
{
	Media::onContainerParameterChanged(p);

	if (p == source)
	{
		bool isFile = source->getValueDataAsEnum<PictureSource>() == Source_File;
		filePath->setEnabled(isFile);
		url->setEnabled(!isFile);
	}
	if (p == source || p == filePath || p == url)
	{
		reloadImage();
	}
}

void PictureMedia::reloadImage()
{
	PictureSource s = source->getValueDataAsEnum<PictureSource>();
	switch (s)
	{

	case Source_File:
	{
		File target = filePath->getFile();

		if (target.existsAsFile() && target.hasFileExtension("jpg;jpeg;png"))
		{
			GenericScopedLock lock(imageLock);
			Image img = ImageFileFormat::loadFrom(target);
			initImage(img);
			shouldRedraw = true;
		}
	}
	break;

	case Source_URL:
	{
		startThread();
	}
	break;
	}
}

void PictureMedia::run()
{
	GenericScopedLock lock(imageLock);
	std::unique_ptr<InputStream> is = URL(url->stringValue()).createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress));

	if (is == nullptr)
	{
		NLOGERROR(niceName, "Couldn't retrieve online picture, are you connected to internet ?");
		return;
	}

	MemoryBlock block;
	is->readIntoMemoryBlock(block);
	MemoryInputStream mis(block, false);
	Image img = ImageFileFormat::loadFrom(mis);
	initImage(img);

}
