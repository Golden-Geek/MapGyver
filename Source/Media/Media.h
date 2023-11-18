/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Media;

class MediaUI :
    public BaseItemUI<Media>
{
public:
    MediaUI(Media* item);
    virtual ~MediaUI();
};


class Media :
    public BaseItem
{
public:
    Media(const String& name = "Media", var params = var());
    virtual ~Media();
    
    unsigned int imageVersion;

    CriticalSection imageLock;
    Image image;
    std::shared_ptr<Image::BitmapData> bitmapData;

    void onContainerParameterChangedInternal(Parameter* p);
    void updateVersion();

    Point<int> getMediaSize();
};
