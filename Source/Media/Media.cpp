/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaUI::MediaUI(Media* item) :
	BaseItemUI(item)
{
}

MediaUI::~MediaUI()
{
}


Media::Media(var params) :
	BaseItem(params.getProperty("name", "Media")),
	objectType(params.getProperty("type", "Media").toString()),
	objectData(params),
	myImage(Image::ARGB, 10,10, true)
{
	saveAndLoadRecursiveData = true;
	//nameCanBeChangedByUser = false;
	//canBeDisabled = false;

	itemDataType = "Media";

}

Media::~Media()
{
}

void Media::onContainerParameterChangedInternal(Parameter* p) {
}


void Media::updateVersion() {
	imageVersion = (imageVersion +1 ) % 65535;
}