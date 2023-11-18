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
	public Component,
	public OpenGLRenderer,
	public KeyListener
{
public:
	ScreenOutput(Screen* parent);
	~ScreenOutput();

	Screen* parentScreen;

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

	void goLive(int screenId);
	void stopLive();

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	void userTriedToCloseWindow() override;

	void createAndLoadShaders();

	bool keyPressed(const KeyPress& key, Component* originatingComponent);

	float estimateWidthHeightRatio(const Array<Point<float>>& corners);

};