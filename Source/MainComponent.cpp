#include "MainIncludes.h"
#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"
#include "Common/CommonIncludes.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    // setSize (800, 600);
    getCommandManager().registerAllCommandsForTarget(this);
    OpenGLManager::getInstance();
    ScreenOutputWatcher::getInstance(); // triggers the creation of the singleton
}

MainContentComponent::~MainContentComponent()
{
    // This shuts down the audio Fixture and clears the audio source.
    //shutdownAudio();

    ScreenOutputWatcher::deleteInstance();
    OpenGLManager::deleteInstance();
}

//==============================================================================
/*
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio Fixture is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio Fixture stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
*/


void MainContentComponent::init()
{
    ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Screens", &ScreenManagerUI::create));
    ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Screen Editor", &ScreenEditorPanel::create));
    ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Medias", &MediaManagerUI::create));

    
    OrganicMainContentComponent::init();

    //getLookAndFeel().setColour(juce::TextButton::textColourOffId, Colour(127,127,127));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, Colour(64,64,64));

}
