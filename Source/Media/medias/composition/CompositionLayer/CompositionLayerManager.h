/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once


class CompositionLayerFactory :
    public Factory<CompositionLayer>
{
public:
    juce_DeclareSingleton(CompositionLayerFactory, true);
    CompositionLayerFactory();
    ~CompositionLayerFactory() {}

};

class CompositionLayerManager :
    public BaseManager<CompositionLayer>
{
public:
    CompositionLayerManager();
    ~CompositionLayerManager();

    void addItemInternal(CompositionLayer* o, var data) override;
    void removeItemInternal(CompositionLayer* o) override;

    void onContainerParameterChanged(Parameter* p) override;

};