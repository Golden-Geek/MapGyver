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
    public Manager<CompositionLayer>
{
public:
    CompositionLayerManager();
    ~CompositionLayerManager();


};