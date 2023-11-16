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
    Media(var params = var());
    virtual ~Media();
    CriticalSection useImageData;

    String objectType;
    var objectData;

    void onContainerParameterChangedInternal(Parameter* p);

    Image myImage;
    std::shared_ptr<Image::BitmapData> bitmapData = nullptr;
    unsigned int imageVersion=0;

    String getTypeString() const override { return objectType; }

    void updateVersion();

    static Media* create(var params) { return new Media(params); }
};
