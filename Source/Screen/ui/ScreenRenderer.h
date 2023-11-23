/*
  ==============================================================================

	ScreenRenderer.h
	Created: 19 Nov 2023 11:44:37am
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class ScreenRenderer :
	public juce::OpenGLRenderer
{
public:
	ScreenRenderer(Screen* screen);
	~ScreenRenderer();

	Screen* screen;

	std::unique_ptr<OpenGLShaderProgram> shader;
	juce::OpenGLFrameBuffer frameBuffer;

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;

	void openGLContextClosing() override;

	void createAndLoadShaders();
};