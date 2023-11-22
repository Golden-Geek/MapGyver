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
    public BaseItem, 
    public OpenGLRenderer
{
public:
    Media(const String& name = "Media", var params = var());
    virtual ~Media();
    
    unsigned int imageVersion;

    CriticalSection imageLock;
    Image image;
    std::shared_ptr<Image::BitmapData> bitmapData;
    OpenGLFrameBuffer frameBuffer;

    void onContainerParameterChangedInternal(Parameter* p);

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;


    Point<int> getMediaSize();
};
