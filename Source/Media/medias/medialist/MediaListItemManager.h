/*
  ==============================================================================

    MediaListItemManager.h
    Created: 18 Feb 2026 8:29:38am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaListItemManager :
    public Manager<MediaListItem>
{
public:
    MediaListItemManager();
    ~MediaListItemManager();

    FloatParameter* thumbSize;
};