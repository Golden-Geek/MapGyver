#include "MainIncludes.h"
#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"
#include "Common/CommonIncludes.h"
#include "Node/NodeIncludes.h"
#include "MainComponent.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{
	// Make sure you set the size of the component after
	// you add any child components.
	// setSize (800, 600);

	//getCommandManager().registerAllCommandsForTarget(this);
	ScreenOutputWatcher::getInstance(); // triggers the creation of the singleton
}

MainContentComponent::~MainContentComponent()
{
	// This shuts down the audio Fixture and clears the audio source.
	//shutdownAudio();


	ScreenOutputWatcher::deleteInstance();
	GlContextHolder::deleteInstance();
}


void MainContentComponent::init()
{
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Screens", &ScreenManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Screen Editor", &ScreenEditorPanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Surface Editor", &SurfaceEditorPanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Medias", &MediaManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Medias (Grid View)", &MediaManagerGridUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Media Preview", &MediaPreviewPanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Online Explorer", &OnlineContentExplorer::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Node Editor", &NodeManagerViewPanel::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Sequence Editor", &TimeMachineView::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Grid Editor", &GridMediaPanel::create));

	OrganicMainContentComponent::init();

	TimeMachineView::getEditingSequenceCustomFunc = [this]() { return MediaManager::getInstance()->editingSequenceMedia != nullptr ? &MediaManager::getInstance()->editingSequenceMedia->sequence : nullptr; };

	//getLookAndFeel().setColour(juce::TextButton::textColourOffId, Colour(127,127,127));
	getLookAndFeel().setColour(juce::TextButton::buttonColourId, Colour(64, 64, 64));
}

void MainContentComponent::afterInit()
{
    OrganicMainContentComponent::afterInit();
    GlContextHolder::getInstance()->setup(this);
}

void MainContentComponent::setupOpenGL()
{
	//do not use organic one
	GlContextHolder::getInstance()->registerOpenGlRenderer(&sharedTextureDispatcher);
}

void MainContentComponent::paint(Graphics& g)
{
	//nothing
	OrganicMainContentComponent::paint(g);
}

void MainContentComponent::SharedTextureDispatcher::newOpenGLContextCreated()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->initGL();
}

void MainContentComponent::SharedTextureDispatcher::renderOpenGL()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->renderGL();
}

void MainContentComponent::SharedTextureDispatcher::openGLContextClosing()
{
	if (SharedTextureManager::getInstanceWithoutCreating() != nullptr) SharedTextureManager::getInstance()->clearGL();
}
