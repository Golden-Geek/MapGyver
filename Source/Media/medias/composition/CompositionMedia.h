/*
  ==============================================================================

    CompositionMedia.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class CompositionMedia :
    public Media
{
public:
    CompositionMedia(var params = var());
    ~CompositionMedia();

    ColorParameter* backgroundColor;

    CompositionLayerManager layers;

    void renderGLInternal() override;

    std::shared_ptr<Graphics> myGraphics = nullptr;
    std::shared_ptr<Graphics> workGraphics = nullptr;
    Image workImage;

    DECLARE_TYPE("Composition")


    HashMap<Media*, int> texturesVersions;

};