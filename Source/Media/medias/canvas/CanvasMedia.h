/*
  ==============================================================================

    CanvasMedia.h
    Created: 11 Feb 2025 3:02:46pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CanvasMedia :
    public ImageMedia
{
public:
    CanvasMedia(var params = var());
    ~CanvasMedia();

	DECLARE_TYPE("Canvas")
};