/*
  ==============================================================================

    AudioManagerEditor.cpp
    Created: 24 Sep 2025 6:51:44pm
    Author:  bkupe

  ==============================================================================
*/

#include "Common/CommonIncludes.h"

AudioManagerEditor::AudioManagerEditor(bool isRoot) :
	GenericControllableContainerEditor(Array<ControllableContainer*>((ControllableContainer*)AudioManager::getInstance()), isRoot),
	selector(AudioManager::getInstance()->am, 0, 32, 0, 32, false, false, false, false)
{
	selector.setSize(100, 300);
	selector.setVisible(!container->editorIsCollapsed);
	addAndMakeVisible(selector);
	resized();
}

AudioManagerEditor::~AudioManagerEditor()
{
}


void AudioManagerEditor::setCollapsed(bool value, bool force, bool animate, bool doNotRebuild)
{
	GenericControllableContainerEditor::setCollapsed(value, force, animate, doNotRebuild);
	selector.setVisible(!container->editorIsCollapsed);
}

void AudioManagerEditor::resizedInternalContent(juce::Rectangle<int>& r)
{
	GenericControllableContainerEditor::resizedInternalContent(r);
	selector.setBounds(r.withHeight(jmin(selector.getHeight(), 300)));
	r.setY(selector.getBottom());
	r.setHeight(0);
}

