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
	HashMap<Media*, std::shared_ptr<OpenGLTexture>> textures;
	HashMap<Media*, int> texturesVersions;
	HashMap<Surface*, GLuint> vertices;
	HashMap<Surface*, int> verticesVersions;
	juce::OpenGLFrameBuffer frameBuffer;



	std::unique_ptr<OpenGLShaderProgram> testShader;

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;

	void drawTest();
	void drawSurface(Surface* s);

	void openGLContextClosing() override;

	void createAndLoadShaders();
};