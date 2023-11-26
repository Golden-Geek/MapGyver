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
}

PictureMedia::~PictureMedia()
{
	stopThread(1000);
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
			initImage(ImageFileFormat::loadFrom(target));
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

	MemoryBlock block;
	is->readIntoMemoryBlock(block);
	MemoryInputStream mis(block, false);
	initImage(ImageFileFormat::loadFrom(mis));

	//if (jpeg.canUnderstand(*is))
	//{
	//	is->setPosition(0);
	//	return;
	//}

	//is->setPosition(0);
	//PNGImageFormat png;
	//if (png.canUnderstand(*is))
	//{
	//	is->setPosition(0);
	//	initImage(png.decodeImage(*is));
	//	return;
	//}

	//GIFImageFormat gif;
	//is->setPosition(0);
	//if (gif.canUnderstand(*is))
	//{
	//	is->setPosition(0);
	//	initImage(gif.decodeImage(*is));
	//	return;
	//}

}
