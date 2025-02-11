/*
  ==============================================================================

    GridMedia.h
    Created: 11 Feb 2025 3:02:19pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridMedia :
    public Media
{
public:
    GridMedia(var params = var());
    ~GridMedia();

	DECLARE_TYPE("Grid")
};