/*
  ==============================================================================

    MediaListItemManager.h
    Created: 18 Feb 2026 8:29:38am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaListItemFactory :
    public Factory<MediaListItem>
{
public:
    juce_DeclareSingleton(MediaListItemFactory, true);
    MediaListItemFactory();
    ~MediaListItemFactory() {}


};

class MediaListItemManager :
    public Manager<MediaListItem>
{
public:
    MediaListItemManager();
    ~MediaListItemManager();

    FloatParameter* thumbSize;
};