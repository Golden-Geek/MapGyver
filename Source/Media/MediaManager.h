/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class SequenceMedia;

class MediaManager :
    public BaseManager<Media>,
    public InspectableSelectionManager::Listener,
    public Inspectable::InspectableListener
{
public:
    juce_DeclareSingleton(MediaManager, true);

    MediaManager();
    ~MediaManager();

    Factory<Media> factory;
    SequenceMedia* editingSequenceMedia;

    IntParameter* gridThumbSize;

    void addItemInternal(Media* o, var data) override;

    void setEditingSequenceMedia(SequenceMedia* sm);

    void inspectablesSelectionChanged() override;
    void inspectableDestroyed(Inspectable* i) override;

    var getJSONData(bool includeNonOverriden = false) override;
    void loadJSONDataManagerInternal(var data) override;

};