/*
  ==============================================================================

	MediaShader.h
	Created: 22 Nov 2023 3:39:16pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class ShaderMedia :
	public Media,
	public Thread
{
public:
	ShaderMedia(var params = var());
	~ShaderMedia();

	enum ShaderType
	{
		ShaderGLSLFile,
		ShaderToyFile,
		ShaderToyURL
	};

	EnumParameter* shaderType;
	FileParameter* shaderFile;
	StringParameter* shaderToyID;
	StringParameter* shaderToyKey;
	ColorParameter* backgroundColor;
	BoolParameter* mouseClick;
	Point2DParameter* mouseInputPos;

	Time lastModificationTime;

	int currentFrame;
	float lastFrameTime;

	SpinLock shaderLock;
	std::unique_ptr<OpenGLShaderProgram> shader;
	GLuint VBO, VAO;

	bool shouldReloadShader;

	String resolutionUniformName;
	String timeUniformName;
	String mouseUniformName;
	String frameUniformName;
	String deltaFrameUniformName;

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
	void reloadShader();
	void loadFragmentShader(const String& fragmentShader);

	void checkForHotReload();

	void run() override;

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
