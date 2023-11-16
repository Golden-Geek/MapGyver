/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class SurfaceManager :
    public BaseManager<Surface>
{
public:
    SurfaceManager();
    ~SurfaceManager();

    void addItemInternal(Surface* o, var data) override;
    void removeItemInternal(Surface* o) override;

    void onContainerParameterChanged(Parameter* p) override;

};