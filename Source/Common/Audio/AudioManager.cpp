/*
  ==============================================================================

	AudioManager.cpp
	Created: 24 Sep 2025 10:57:48am
	Author:  bkupe

  ==============================================================================
*/

#include "Common/CommonIncludes.h"
#include "AudioManager.h"

juce_ImplementSingleton(AudioManager)

AudioManager::AudioManager() :
	ControllableContainer("Audio Manager"),
	inputsCC("Inputs"),
	outputsCC("Outputs"),
	isFillingIO(false),
	graphIDIncrement(GRAPH_UNIQUE_ID_START)
{
	favoriteTrigger = addTrigger("Set as Favorite", "Sets the current audio setup as favorite");
	fillIOFromSetupTrigger = addTrigger("Fill IO from Setup", "Fills the audio inputs and outputs from the favorite setup");

	masterGain = addFloatParameter("Master Gain", "Master Gain", 1, 0, 2);
	masterVolume = addFloatParameter("Master Volume", "Master Volume", 1, 0, 2);

	inputsCC.userCanAddControllables = true;
	inputsCC.customUserCreateControllableFunc = std::bind(&AudioManager::createVolumeControl, this, std::placeholders::_1);
	outputsCC.userCanAddControllables = true;
	outputsCC.userAddControllablesFilters = FloatParameter::getTypeStringStatic();

	addChildControllableContainer(&inputsCC);
	addChildControllableContainer(&outputsCC);

	saveAndLoadRecursiveData = true;

	am.initialiseWithDefaultDevices(0, 2);

	am.addAudioCallback(&player);
	am.addChangeListener(this);

	std::unique_ptr<AudioProcessorGraph::AudioGraphIOProcessor> procIn(new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
	std::unique_ptr<AudioProcessorGraph::AudioGraphIOProcessor> procOut(new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
	graph.addNode(std::move(procIn), AUDIO_INPUT_GRAPH_ID);
	graph.addNode(std::move(procOut), AUDIO_OUTPUT_GRAPH_ID);

	auto iProc = std::unique_ptr<MixerProcessor>(new MixerProcessor(this, true));
	inputMixer = iProc.get();

	auto oProc = std::unique_ptr<MixerProcessor>(new MixerProcessor(this, false));
	outputMixer = oProc.get();

	graph.addNode(std::move(iProc), AUDIO_INPUTMIXER_GRAPH_ID);
	graph.addNode(std::move(oProc), AUDIO_OUTPUTMIXER_GRAPH_ID);
	player.setProcessor(&graph);


	fillIOFromSetup();

	if (!Engine::mainEngine->isLoadingFile) initAudio();
}

AudioManager::~AudioManager()
{
	graph.suspendProcessing(true);
	player.setProcessor(nullptr);
	am.closeAudioDevice();
}

void AudioManager::childStructureChanged(ControllableContainer* cc)
{
	ControllableContainer::childStructureChanged(cc);

	if (cc == &inputsCC || cc == &outputsCC)
	{
		initAudio();
	}
}

void AudioManager::createVolumeControl(ControllableContainer* parent)
{
	FloatParameter* p = parent->addFloatParameter("Volume " + String(parent->controllables.size() + 1), "Volume", 1, 0, 2);
	p->saveValueOnly = false;
	p->userCanChangeName = true;
}

void AudioManager::checkFavoriteSetupAndApplyIfNeeded()
{
	if (checkFavoriteIsAvailable())
	{
		am.initialise(0, 2, nullptr, true, "", &favoriteSetup);
	}
}

bool AudioManager::checkFavoriteIsAvailable()
{
	const OwnedArray<AudioIODeviceType>& availableDevices = am.getAvailableDeviceTypes();
	bool foundInput = false;
	bool foundOutput = false;
	for (auto& type : availableDevices)
	{
		StringArray devices = type->getDeviceNames();
		for (auto& d : devices)
		{
			if (d == favoriteSetup.inputDeviceName)
				foundInput = true;
			if (d == favoriteSetup.outputDeviceName)
				foundOutput = true;
		}
	}

	return foundInput && foundOutput;
}

void AudioManager::setAsFavoriteSetup()
{
	am.getAudioDeviceSetup(favoriteSetup);
	NLOG(niceName, "Set favorite audio setup:\nInput " + favoriteSetup.inputDeviceName
		+ "\nOutput: " + favoriteSetup.outputDeviceName
		+ "\nSample Rate: " + String(favoriteSetup.sampleRate)
		+ "\nBuffer Size: " + String(favoriteSetup.bufferSize));
}

void AudioManager::fillIOFromSetup()
{
	isFillingIO = true;

	AudioDeviceManager::AudioDeviceSetup setup;
	am.getAudioDeviceSetup(setup);

	StringArray inputNames = am.getCurrentAudioDevice()->getInputChannelNames();
	StringArray outputNames = am.getCurrentAudioDevice()->getOutputChannelNames();

	inputsCC.clear();
	outputsCC.clear();
	int numInputs = setup.inputChannels.countNumberOfSetBits();
	int numOutputs = setup.outputChannels.countNumberOfSetBits();
	for (int i = 0; i < numInputs; i++)
	{
		createVolumeControl(&inputsCC);
		((FloatParameter*)inputsCC.controllables.getLast())->setNiceName(inputNames[i]);

	}
	for (int i = 0; i < numOutputs; i++)
	{
		createVolumeControl(&outputsCC);
		((FloatParameter*)outputsCC.controllables.getLast())->setNiceName(outputNames[i]);
	}

	isFillingIO = false;
	initAudio();
}

int AudioManager::getNumUserInputs()
{
	return inputsCC.controllables.size();
}

int AudioManager::getNumUserOutputs()
{
	return outputsCC.controllables.size();
}

void AudioManager::onContainerTriggerTriggered(Trigger* t)
{
	if (t == favoriteTrigger)
		setAsFavoriteSetup();
	else if (t == fillIOFromSetupTrigger)
		fillIOFromSetup();
}

void AudioManager::changeListenerCallback(ChangeBroadcaster* source)
{
	NLOG(niceName, "Audio device changed");

	if (source == &am)
	{
		// Audio device changed

		if(outputsCC.controllables.size() == 0 && inputsCC.controllables.size() == 0)
			fillIOFromSetup();

		//if favorite setup is empty, fill it
		if (favoriteSetup.inputDeviceName.isEmpty() && favoriteSetup.outputDeviceName.isEmpty())
		{
			setAsFavoriteSetup();
		}
		else
		{
			checkFavoriteSetupAndApplyIfNeeded();
		}


	}

	initAudio();
}

void AudioManager::initAudio()
{

	if (isFillingIO || isCurrentlyLoadingData) return;

	if( getNumUserInputs() == 0 && getNumUserOutputs() == 0)
	{
		NLOGWARNING(niceName, "No audio inputs or outputs defined, cannot init audio.");
		return;
	}

	graph.suspendProcessing(true);

	AudioDeviceManager::AudioDeviceSetup setup;
	am.getAudioDeviceSetup(setup);

	//get num output and input channels from device
	int numInputChannels = setup.inputChannels.countNumberOfSetBits();
	int numOutputChannels = setup.outputChannels.countNumberOfSetBits();

	graph.setPlayConfigDetails(numInputChannels, numOutputChannels, setup.sampleRate, setup.bufferSize);
	graph.prepareToPlay(setup.sampleRate, setup.bufferSize);

	
	inputMixer->setPlayConfigDetails(numInputChannels, getNumUserInputs()	, setup.sampleRate, setup.bufferSize);

	graph.disconnectNode(AUDIO_OUTPUTMIXER_GRAPH_ID);
	outputMixer->setPlayConfigDetails(getNumUserOutputs(), numOutputChannels, setup.sampleRate, setup.bufferSize);

	if (numOutputChannels > 0)
	{
		for (int i = 0; i < getNumUserInputs(); ++i)
		{
			graph.addConnection(AudioProcessorGraph::Connection({ AUDIO_INPUT_GRAPH_ID, i }, { AUDIO_INPUTMIXER_GRAPH_ID, i % numInputChannels }));
		}

		for (int i = 0; i < getNumUserOutputs(); ++i)
		{
			graph.addConnection(AudioProcessorGraph::Connection({ AUDIO_OUTPUTMIXER_GRAPH_ID, i }, { AUDIO_OUTPUT_GRAPH_ID, i % numOutputChannels }));
		}
	}

	graph.rebuild();

	amListeners.call(&AudioManagerListener::audioSetupChanged);
	

	graph.suspendProcessing(false);

}

int AudioManager::getUniqueNodeGraphID()
{
	return graphIDIncrement++;
}

InspectableEditor* AudioManager::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new AudioManagerEditor(isRoot);
}

juce::var AudioManager::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData();

	var setupData(new DynamicObject());
	setupData.getDynamicObject()->setProperty("inputDevice", favoriteSetup.inputDeviceName);
	setupData.getDynamicObject()->setProperty("outputDevice", favoriteSetup.outputDeviceName);
	setupData.getDynamicObject()->setProperty("sampleRate", favoriteSetup.sampleRate);
	setupData.getDynamicObject()->setProperty("bufferSize", favoriteSetup.bufferSize);
	setupData.getDynamicObject()->setProperty("useDefaultInputChannels", favoriteSetup.useDefaultInputChannels);
	setupData.getDynamicObject()->setProperty("useDefaultOutputChannels", favoriteSetup.useDefaultOutputChannels);
	setupData.getDynamicObject()->setProperty("inputChannels", favoriteSetup.inputChannels.toInt64());
	setupData.getDynamicObject()->setProperty("outputChannels", (int)favoriteSetup.outputChannels.toInt64());
	data.getDynamicObject()->setProperty("favoriteSetup", setupData);


	var lastSetupData(new DynamicObject());
	AudioDeviceManager::AudioDeviceSetup lastSetup;
	am.getAudioDeviceSetup(lastSetup);
	lastSetupData.getDynamicObject()->setProperty("inputDevice", lastSetup.inputDeviceName);
	lastSetupData.getDynamicObject()->setProperty("outputDevice", lastSetup.outputDeviceName);
	lastSetupData.getDynamicObject()->setProperty("sampleRate", lastSetup.sampleRate);
	lastSetupData.getDynamicObject()->setProperty("bufferSize", lastSetup.bufferSize);
	lastSetupData.getDynamicObject()->setProperty("useDefaultInputChannels", lastSetup.useDefaultInputChannels);
	lastSetupData.getDynamicObject()->setProperty("useDefaultOutputChannels", lastSetup.useDefaultOutputChannels);
	lastSetupData.getDynamicObject()->setProperty("inputChannels", lastSetup.inputChannels.toInt64());
	lastSetupData.getDynamicObject()->setProperty("outputChannels", (int)lastSetup.outputChannels.toInt64());


	data.getDynamicObject()->setProperty("lastSetup", lastSetupData);
	return data;
}

void AudioManager::loadJSONDataInternal(juce::var data)
{
	var setupData = data.getProperty("favoriteSetup", var());
	if (!setupData.isVoid())
	{
		favoriteSetup.inputDeviceName = setupData.getProperty("inputDevice", "").toString();
		favoriteSetup.outputDeviceName = setupData.getProperty("outputDevice", "").toString();
		favoriteSetup.sampleRate = (double)setupData.getProperty("sampleRate", 44100.0);
		favoriteSetup.bufferSize = (int)setupData.getProperty("bufferSize", 512);
		favoriteSetup.useDefaultInputChannels = (bool)setupData.getProperty("useDefaultInputChannels", true);
		favoriteSetup.useDefaultOutputChannels = (bool)setupData.getProperty("useDefaultOutputChannels", true);
		favoriteSetup.inputChannels = (int64)setupData.getProperty("inputChannels", 0);
		favoriteSetup.outputChannels = (int64)setupData.getProperty("outputChannels", 0);

		String s = am.initialise(0, 2, nullptr, true, "", &favoriteSetup);
		if (!s.isEmpty())
		{
			NLOGWARNING(niceName, "Error applying favorite audio setup: " + s + ", trying to load last used setup");
			var lastSetupData = data.getProperty("lastSetup", var());
			if (!lastSetupData.isVoid())
			{
				AudioDeviceManager::AudioDeviceSetup lastSetup;
				lastSetup.inputDeviceName = lastSetupData.getProperty("inputDevice", "").toString();
				lastSetup.outputDeviceName = lastSetupData.getProperty("outputDevice", "").toString();
				lastSetup.sampleRate = (double)lastSetupData.getProperty("sampleRate", 44100.0);
				lastSetup.bufferSize = (int)lastSetupData.getProperty("bufferSize", 512);
				lastSetup.useDefaultInputChannels = (bool)lastSetupData.getProperty("useDefaultInputChannels", true);
				lastSetup.useDefaultOutputChannels = (bool)lastSetupData.getProperty("useDefaultOutputChannels", true);
				lastSetup.inputChannels = (int64)lastSetupData.getProperty("inputChannels", 0);
				lastSetup.outputChannels = (int64)lastSetupData.getProperty("outputChannels", 0);
				s = am.initialise(0, 2, nullptr, true, "", &lastSetup);
				if (!s.isEmpty())
				{
					NLOGWARNING(niceName, "Error applying last used audio setup: " + s + ", falling back to default audio device");
					am.initialiseWithDefaultDevices(0, 2);
				}
			}
			else
			{
				NLOGWARNING(niceName, "No last used audio setup found, falling back to default audio device");
				am.initialiseWithDefaultDevices(0, 2);
			}

		}
	}

}

void AudioManager::afterLoadJSONDataInternal()
{
	for (auto& c : inputsCC.controllables)
	{
		if (auto* p = dynamic_cast<FloatParameter*>(c))
		{
			p->saveValueOnly = false;
			p->userCanChangeName = true;
		}
	}

	for (auto& c : outputsCC.controllables)
	{
		if (auto* p = dynamic_cast<FloatParameter*>(c))
		{
			p->saveValueOnly = false;
			p->userCanChangeName = true;
		}
	}

	initAudio();
}





MixerProcessor::MixerProcessor(AudioManager* m, bool isInput) :
	AudioProcessor(),
	audioManager(m),
	isInput(isInput)
{
	gainsCC = isInput ? &audioManager->inputsCC : &audioManager->outputsCC;
}

void MixerProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
}

void MixerProcessor::releaseResources()
{

}

void MixerProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	int numGains = isInput ? audioManager->getNumUserInputs() : audioManager->getNumUserOutputs();
	while (prevGains.size() < numGains) prevGains.add(0);
	prevGains.removeRange(numGains, prevGains.size() - numGains);

	for (int i = 0; i < buffer.getNumChannels() && i < numGains; i++)
	{
		float newGain = ((FloatParameter*)gainsCC->controllables[i])->floatValue();
		buffer.applyGainRamp(i, 0, buffer.getNumSamples(), prevGains[i], newGain);
		prevGains.set(i, newGain);
	}
}

double MixerProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

bool MixerProcessor::acceptsMidi() const
{
	return false;
}

bool MixerProcessor::producesMidi() const
{
	return false;
}

AudioProcessorEditor* MixerProcessor::createEditor()
{
	return nullptr;
}

bool MixerProcessor::hasEditor() const
{
	return false;
}

int MixerProcessor::getNumPrograms()
{
	return 0;
}

int MixerProcessor::getCurrentProgram()
{
	return 0;
}

void MixerProcessor::setCurrentProgram(int index)
{
}

const String MixerProcessor::getProgramName(int index)
{
	return String();
}

void MixerProcessor::changeProgramName(int index, const String& newName)
{
}

void MixerProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void MixerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}
