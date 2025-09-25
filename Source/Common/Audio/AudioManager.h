/*
  ==============================================================================

	AudioManager.h
	Created: 24 Sep 2025 10:57:48am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#define AUDIO_INPUT_GRAPH_ID AudioProcessorGraph::NodeID(1)
#define AUDIO_OUTPUT_GRAPH_ID AudioProcessorGraph::NodeID(2)
#define AUDIO_INPUTMIXER_GRAPH_ID AudioProcessorGraph::NodeID(3)
#define AUDIO_OUTPUTMIXER_GRAPH_ID AudioProcessorGraph::NodeID(4)

class AudioManager;

class MixerProcessor :
	public juce::AudioProcessor
{
public:
	MixerProcessor(AudioManager* audioModule, bool isInput);

	AudioManager* audioManager;
	bool isInput;
	ControllableContainer* gainsCC;
	juce::Array<float> prevGains;

	// Hérité via AudioProcessor
	virtual const juce::String getName() const override { return isInput ? "Input Mixer" : "Output Mixer"; }
	virtual void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
	virtual void releaseResources() override;
	virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
	virtual double getTailLengthSeconds() const override;
	virtual bool acceptsMidi() const override;
	virtual bool producesMidi() const override;
	virtual juce::AudioProcessorEditor* createEditor() override;
	virtual bool hasEditor() const override;
	virtual int getNumPrograms() override;
	virtual int getCurrentProgram() override;
	virtual void setCurrentProgram(int index) override;
	virtual const juce::String getProgramName(int index) override;
	virtual void changeProgramName(int index, const juce::String& newName) override;
	virtual void getStateInformation(juce::MemoryBlock& destData) override;
	virtual void setStateInformation(const void* data, int sizeInBytes) override;
};

class AudioManager :
	public ControllableContainer,
	public juce::ChangeListener
{
public:
	juce_DeclareSingleton(AudioManager, true);

	AudioManager();
	~AudioManager() override;

	juce::AudioDeviceManager am;
	juce::AudioProcessorPlayer player;
	juce::AudioProcessorGraph graph;

	juce::AudioDeviceManager::AudioDeviceSetup favoriteSetup;

	Trigger* favoriteTrigger;
	Trigger* fillIOFromSetupTrigger;

	FloatParameter* masterGain;
	FloatParameter* masterVolume;

	MixerProcessor* inputMixer;
	MixerProcessor* outputMixer;

	ControllableContainer inputsCC;
	ControllableContainer outputsCC;

	bool isFillingIO;

	void childStructureChanged(ControllableContainer* cc) override;

	void createVolumeControl(ControllableContainer* parent);

	void checkFavoriteSetupAndApplyIfNeeded();
	bool checkFavoriteIsAvailable();
	void setAsFavoriteSetup();
	void fillIOFromSetup();

	int getNumUserInputs();
	int getNumUserOutputs();

	void onContainerTriggerTriggered(Trigger* t) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	void initAudio();

	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

	juce::var getJSONData(bool includeNonOverriden = true) override;
	void loadJSONDataInternal(juce::var data) override;
	void afterLoadJSONDataInternal() override;

	class AudioManagerListener
	{
	public:
		virtual void audioSetupChanged() {}
	};

	juce::ListenerList<AudioManagerListener> amListeners;

	void addAudioManagerListener(AudioManagerListener* l) { amListeners.add(l); }
	void removeAudioManagerListener(AudioManagerListener* l) { amListeners.remove(l); }
};