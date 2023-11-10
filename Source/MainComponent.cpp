#include "JuceHeader.h"
#include "MainComponent.h"
#include "RMPEngine.h"

#include "Definitions/Screen/ScreenManagerUI.h"


//==============================================================================
MainContentComponent::MainContentComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    // setSize (800, 600);
    getCommandManager().registerAllCommandsForTarget(this);
}

MainContentComponent::~MainContentComponent()
{
    // This shuts down the audio Fixture and clears the audio source.
    //shutdownAudio();
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

    /*
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Interfaces", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Channels config", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Fixture Types", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Fixtures", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Groups", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Presets", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Timing Presets", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Curve Presets", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Cuelists", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Programmers", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Effects", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Carousels", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Mappers", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Multiplicators", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Layouts", "Lists");

    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Encoders", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Encoders Overview", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Input Panel", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Fixture Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Group Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Preset Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Cuelist Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Effect Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Carousel Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Mapper Grid", "Grids");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Virtual buttons", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Virtual buttons grid", "Playback");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Virtual faders", "Lists");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Virtual faders grid", "Playback");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Conductor Infos", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Color Picker", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("DMX Tester", "Panels");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Layout Viewer", "Panels");

    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Outliner", "");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Dashboard", "");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Help", "");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("Parrots", "Organic Tools");
    ShapeShifterManager::getInstance()->isInViewSubMenu.set("The Detective", "Organic Tools");

    std::sort(ShapeShifterFactory::getInstance()->defs.begin(), ShapeShifterFactory::getInstance()->defs.end(),
        [](ShapeShifterDefinition* a, ShapeShifterDefinition* b) { return a->contentName < b->contentName; });

    */
    OrganicMainContentComponent::init();

    //getLookAndFeel().setColour(juce::TextButton::textColourOffId, Colour(127,127,127));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, Colour(64,64,64));

}
