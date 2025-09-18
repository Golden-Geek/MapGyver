/*
  ==============================================================================

	ColorMedia.h
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ColorMedia :
	public Media
{
public:
	ColorMedia(var params = var());
	~ColorMedia();

	ColorParameter* color;

	void clearItem() override;

	void renderGLInternal() override;

	Point<int> getDefaultMediaSize() override;
	DECLARE_TYPE("Solid Color")
};