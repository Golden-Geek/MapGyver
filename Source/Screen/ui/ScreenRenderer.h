/*
  ==============================================================================

	ScreenRenderer.h
	Created: 19 Nov 2023 11:44:37am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include <atomic>

class ScreenRenderer :
	public juce::OpenGLRenderer
{
public:
	ScreenRenderer(Screen* screen);
	~ScreenRenderer();

	Screen* screen;

	std::unique_ptr<OpenGLShaderProgram> shader;
	juce::OpenGLFrameBuffer frameBuffer;
	std::atomic<int> requestedTextureWidth;
	std::atomic<int> requestedTextureHeight;

	void regenerateTextures();

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;

	void openGLContextClosing() override;

	void createAndLoadShaders();
	void updateFrameBufferSize();
};
