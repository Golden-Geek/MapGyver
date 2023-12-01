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

    Point2DParameter* resolution;
 
    CompositionLayerManager layers;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    
    void renderGLInternal() override;

    std::shared_ptr<Graphics> myGraphics = nullptr;
    std::shared_ptr<Graphics> workGraphics = nullptr;
    Image workImage;

    void updateImagesSize();
    bool imageNeedRepaint = true;

    Point<int> getMediaSize() override;

    
    DECLARE_TYPE("Composition")

    void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

    HashMap<Media*, int> texturesVersions;

};