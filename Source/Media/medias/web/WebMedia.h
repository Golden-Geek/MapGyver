/*
  ==============================================================================

    WebMedia.h
    Created: 11 Feb 2025 3:03:12pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class WebMedia :
    public Media
{
public:
    WebMedia(var params = var());
    ~WebMedia();

	DECLARE_TYPE("Web")
};