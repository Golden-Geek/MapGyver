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
	public IInteractableMedia,
	public MediaTarget,
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
		ShaderISFFile,
		ShaderISFURL
	};

	struct UniformInfo
	{
		String name;
		Controllable::Type type;
		var minVal;
		var maxVal;
		var defaultVal;
	};

	Array<UniformInfo> detectedUniforms;

	const String defaultShaderToyID = "4dX3Rn";
	const String defaultISFID = "5e7a80077c113618206de928";

	EnumParameter* shaderType;
	FileParameter* shaderFile;
	StringParameter* onlineShaderID;
	StringParameter* shaderToyKey;
	IntParameter* fps;
	ColorParameter* backgroundColor;
	BoolParameter* keepOfflineCache;
	BoolParameter* shaderLoaded;


	ControllableContainer sourceMedias;

	FloatParameter* speed;
	Trigger* resetTime;
	bool useMouse4D;
	int useResolution3D;
	BoolParameter* mouseClick;
	Point2DParameter* mouseInputPos;

	Time lastModificationTime;

	int currentFrame;
	double lastFrameTime;
	double firstFrameTime = 0;

	long timeAtLastFrame = 0;
	double currentTime = 0;

	//SpinLock shaderLock;
	std::unique_ptr<OpenGLShaderProgram> shader;
	GLuint VBO, VAO;

	bool autoLoadShader;
	bool shouldReloadShader;
	String fragmentShaderToLoad;
	String shaderOfflineData; //for online shader, store the data to be able to reload it offline
	bool isLoadingShader;

	String textureUniformName;
	String resolutionUniformName;
	String normCoordUniformName;
	String timeUniformName;
	String timeDeltaUniformName;
	String mouseUniformName;
	String frameUniformName;

	StringArray isfTextureNames;

	const float vertices[24] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f
		};

	bool isUsingMedia(Media* m) override;

	void onContainerTriggerTriggered(Trigger* t) override;
	void onContainerParameterChangedInternal(Parameter* p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void initGLInternal() override;
	void preRenderGLInternal() override;
	void renderGLInternal() override;
	void reloadShader();
	void loadFragmentShader(const String& fragmentShader);
	String insertShaderIncludes(const String& fragmentShader);
	String parseUniforms(const String& fragmentShader);


	void showUniformControllableMenu(ControllableContainer* cc);
	void addUniformControllable(UniformInfo info);

	Point<float> getMediaShaderPosition(const MouseEvent& e, Rectangle<int> canvasRect);

	void sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect) override;
	void sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
	void sendKeyPressed(const KeyPress& key) override;


	void run() override;

	String getMediaContentName() const override;

	var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataItemInternal(var data) override;

	DECLARE_TYPE("Shader")

};

