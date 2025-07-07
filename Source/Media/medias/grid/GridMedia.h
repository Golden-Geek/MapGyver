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

	GridLayerGroupManager layerGroupManager;
	GridColumnManager columnManager;
	GridClipManager clipManager;

    void createDefaultSetup();

	DECLARE_TYPE("Grid")
};