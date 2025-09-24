/*
  ==============================================================================

    AudioManagerEditor.h
    Created: 24 Sep 2025 6:51:44pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class AudioManagerEditor :
    public GenericControllableContainerEditor
{
public:
    AudioManagerEditor(bool isRoot);
    ~AudioManagerEditor() override;

    void setCollapsed(bool value, bool force = false, bool animate = true, bool doNotRebuild = false) override;
    void resizedInternalContent(juce::Rectangle<int>& r) override;

    AudioDeviceSelectorComponent selector;
};