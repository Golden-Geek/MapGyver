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
		ShaderToyURL,
		ShaderISFFile
	};

	EnumParameter* shaderType;
	FileParameter* shaderFile;
	StringParameter* shaderToyID;
	StringParameter* shaderToyKey;
	IntParameter* fps;
	ColorParameter* backgroundColor;
	

	bool useMouse4D;
	int useResolution3D;
	BoolParameter* mouseClick;
	Point2DParameter* mouseInputPos;

	GenericControllableManager customParamsManager;

	Time lastModificationTime;

	int currentFrame;
	float lastFrameTime;

	//SpinLock shaderLock;
	std::unique_ptr<OpenGLShaderProgram> shader;
	GLuint VBO, VAO;

	bool shouldReloadShader;
	bool isLoadingShader;

	String resolutionUniformName;
	String normCoordUniformName;
	String timeUniformName;
	String timeDeltaUniformName;
	String mouseUniformName;
	String frameUniformName;

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
