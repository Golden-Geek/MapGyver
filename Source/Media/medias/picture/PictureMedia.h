/*
  ==============================================================================

	PictureMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class PictureMedia :
	public ImageMedia,
	public Thread
{
public:
	PictureMedia(var params = var());
	~PictureMedia();

	enum PictureSource { Source_File, Source_URL};
	EnumParameter* source;
	FileParameter* filePath;
	StringParameter* url;
	Trigger* convertToLocal;

	void onContainerTriggerTriggered(Trigger* t) override;
	void onContainerParameterChanged(Parameter* p) override;

	void reloadImage();
	void run() override;

	DECLARE_TYPE("Picture")
};