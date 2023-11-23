/*
  ==============================================================================

	MediaShader.h
	Created: 22 Nov 2023 3:39:16pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class ShaderMedia :
	public Media
{
public:
	ShaderMedia(var params = var());
	~ShaderMedia();

	FileParameter* shaderFile;
	Time lastModificationTime;

	SpinLock shaderLock;
	std::unique_ptr<OpenGLShaderProgram> shader;
	GLuint VBO, VAO;

	bool shouldReloadShader;

	const float vertices[24] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f
		};

	void onContainerParameterChangedInternal(Parameter* p) override;

	void initGL() override;
	void renderGL() override;
	void loadShader();

	DECLARE_TYPE("Shader")
};

class ShaderCheckTimer :
	public Timer
{
public:
	juce_DeclareSingleton(ShaderCheckTimer, true);
	ShaderCheckTimer() { startTimer(500); }
	~ShaderCheckTimer() {}

	void timerCallback() override;
};
