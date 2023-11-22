/*
  ==============================================================================

	PictureMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class PictureMedia :
	public ImageMedia
{
public:
	PictureMedia(var params = var());
	~PictureMedia();

	FileParameter* filePath;

	void onContainerParameterChanged(Parameter* p) override;


	DECLARE_TYPE("Picture")
};