/*
  ==============================================================================

    MediaComposition.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaComposition :
    public Media,
    public Thread
{
public:
    MediaComposition(var params = var());
    ~MediaComposition();

    Point2DParameter* resolution;
    FloatParameter* fps;

    CompositionLayerManager layers;

    void clearItem() override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    
    void run() override;

    void repaintImage();

    std::shared_ptr<Graphics> myGraphics = nullptr;
    std::shared_ptr<Graphics> workGraphics = nullptr;
    Image workImage;

    void updateImagesSize();
    bool imageNeedRepaint = true;

    String getTypeString() const override { return "Composition"; }
    static MediaComposition* create(var params) { return new MediaComposition(); };
    void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

    HashMap<Media*, int> texturesVersions;

    //virtual MediaUI* createUI() {return new MediaComposition(); };
};