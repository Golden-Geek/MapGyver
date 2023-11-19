/*
  ==============================================================================

	ScreenOutput.h
	Created: 9 Nov 2023 8:51:23pm
	Author:  rosta

  ==============================================================================
*/

#pragma once

class Screen;
class Media;

class ScreenOutput :
	public InspectableContentComponent,
	public OpenGLRenderer,
	public KeyListener
{
public:
	ScreenOutput(Screen* parent);
	~ScreenOutput();

	Screen* screen;

	Point2DParameter* closestHandle;
	Surface* manipSurface;
	Array<Point<float>> posAtMouseDown;

	Array<Point2DParameter*> overlapHandles;

	std::unique_ptr<OpenGLShaderProgram> shader;
	juce::OpenGLContext openGLContext;
	Point<float> mousePos;
	Component* previousParent = nullptr;
	bool isLive = false;

	HashMap<Media*, std::shared_ptr<OpenGLTexture>> textures;
	HashMap<Media*, int> texturesVersions;
	HashMap<Surface*, GLuint> vertices;
	HashMap<Surface*, int> verticesVersions;

	void update();

	void paint(Graphics& g) override;
	void paintOverChildren(Graphics&) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseMove(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;

	Point<float> getRelativeMousePos();
	Point<float> getRelativeScreenPos(Point<int> screenPos);
	Point<int> getPointOnScreen(Point<float> pos);


	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	void userTriedToCloseWindow() override;

	void createAndLoadShaders();

	bool keyPressed(const KeyPress& key, Component* originatingComponent);
};


class ScreenOutputWatcher :
	public ScreenManager::AsyncListener,
	public ContainerAsyncListener,
	public EngineListener
{
public:
	juce_DeclareSingleton(ScreenOutputWatcher, false);
	
	ScreenOutputWatcher();
	~ScreenOutputWatcher();

	OwnedArray<ScreenOutput> outputs;

	void updateOutput(Screen* s);

	ScreenOutput* getOutputForScreen(Screen* s);

	void newMessage(const ScreenManager::ManagerEvent& e) override;
	void newMessage(const ContainerAsyncEvent& e) override;

	void startLoadFile() override;
	void endLoadFile() override;
};