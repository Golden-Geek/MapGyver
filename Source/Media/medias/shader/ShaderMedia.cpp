/*
  ==============================================================================

	MediaShader.cpp
	Created: 22 Nov 2023 3:39:16pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

juce_ImplementSingleton(ShaderCheckTimer);

//implement all functions from .h

ShaderMedia::ShaderMedia(var params) :
	Media(getTypeString(), params, true),
	Thread("ShaderToy Loader"),
	shouldReloadShader(false),
	lastModificationTime(0),
	currentFrame(0),
	lastFrameTime(0),
	VBO(0),
	VAO(0)
{
	shaderType = addEnumParameter("Shader Type", "Type Shader to load");
	shaderType->addOption("Shader GLSL File", ShaderGLSLFile)->addOption("ShaderToy File", ShaderToyFile)->addOption("ShaderToy URL", ShaderToyURL);

	shaderFile = addFileParameter("Fragment Shader", "Fragment Shader");
	shaderToyID = addStringParameter("ShaderToy ID", "ID of the shader toy. It's the last part of the URL when viewing it on the website", "tsXBzS", false);
	shaderToyKey = addStringParameter("ShaderToy Key", "Key of the shader toy. It's the last part of the URL when viewing it on the website", "Bd8jRr", false);

	backgroundColor = addColorParameter("Background Color", "Background Color", Colours::black);

	mouseClick = addBoolParameter("Mouse Click", "Simulates mouse click, for shader toy", false);
	mouseInputPos = addPoint2DParameter("Mouse Input Pos", "Mouse Input Pos");
	mouseInputPos->setBounds(0, 0, 1, 1);

	alwaysRedraw = true;
}

ShaderMedia::~ShaderMedia()
{
	stopThread(1000);
}

void ShaderMedia::onContainerParameterChangedInternal(Parameter* p)
{
	Media::onContainerParameterChangedInternal(p);
	if (p == shaderType)
	{
		ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();
		shaderFile->setEnabled(st == ShaderGLSLFile || st == ShaderToyFile);
		shaderToyID->setEnabled(st == ShaderToyURL);
		shaderToyKey->setEnabled(st == ShaderToyURL);
	}
	if (p == shaderFile || p == shaderToyID || p == shaderToyKey) shouldReloadShader = true;
}

void ShaderMedia::initGL()
{
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
}

void ShaderMedia::renderGL()
{
	if (shouldReloadShader) reloadShader();

	GenericScopedLock lock(shaderLock);
	if (shader == nullptr) return;

	Point<int> size = getMediaSize();

	shader->use();

	float t = Time::getMillisecondCounter() / 1000.0f;
	float delta = t - lastFrameTime;

	Point<float> mousePos;
	if(mouseClick->boolValue()) mousePos = mouseInputPos->getPoint() * Point<float>(size.x, size.y);

	//Set uniforms
	if (resolutionUniformName.isNotEmpty())	shader->setUniform(resolutionUniformName.toStdString().c_str(), size.x, size.y);
	if (timeUniformName.isNotEmpty()) shader->setUniform(timeUniformName.toStdString().c_str(), (float)t);
	if (deltaFrameUniformName.isNotEmpty()) shader->setUniform(deltaFrameUniformName.toStdString().c_str(), delta);
	if (frameUniformName.isNotEmpty()) shader->setUniform(frameUniformName.toStdString().c_str(), currentFrame);
	if (mouseUniformName.isNotEmpty()) shader->setUniform(mouseUniformName.toStdString().c_str(), mousePos.x, mousePos.y, mouseClick->floatValue(), 0.f);

	//Draw
	Init2DViewport(size.x, size.y);

	Colour bgColor = backgroundColor->getColor();
	glClearColor(bgColor.getFloatRed(), bgColor.getFloatGreen(), bgColor.getFloatBlue(), bgColor.getFloatAlpha());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);

	currentFrame++;
}

void ShaderMedia::reloadShader()
{
	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	bool isFile = st == ShaderGLSLFile || st == ShaderToyFile;
	if (isFile)
	{
		File sf = shaderFile->getFile();
		String s = "";
		if (sf.existsAsFile())
		{
			s = shaderFile->getFile().loadFileAsString();
			lastModificationTime = sf.getLastModificationTime();
		}

		loadFragmentShader(s);
	}
	else
	{
		startThread();
	}

	shouldReloadShader = false;
}

void ShaderMedia::loadFragmentShader(const String& fragmentShader)
{
	GenericScopedLock lock(shaderLock);
	//unload shader
	shader.reset();


	if (fragmentShader.isEmpty()) return;

	shader.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));

	String fShader = fragmentShader.contains("#version") ? "" : "#version 330\n";

	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	if (st == ShaderToyFile || st == ShaderToyURL)
	{
		fShader = R"(
			#ifdef GL_ES
			precision mediump float;
			#endif

			uniform vec2 iResolution;
			uniform int iFrame;
			uniform float iTime;
			uniform float iDeltaTime;
			uniform vec4 iMouse;
	
			)" +
			fragmentShader +
			R"(

			void main()
			{
				vec4 col = vec4(0.0, 0.0,0.0,1.0);
				mainImage(col, gl_FragCoord.xy);      
				gl_FragColor = col;
			}
			)";

		resolutionUniformName = "iResolution";
		timeUniformName = "iTime";
		mouseUniformName = "iMouse";
		frameUniformName = "iFrame";
		deltaFrameUniformName = "iDeltaFrame";

	}
	else
	{
		resolutionUniformName = "u_resolution";
		timeUniformName = "u_time";
		mouseUniformName = "u_mouse";
		frameUniformName = "";
		deltaFrameUniformName = "";

		fShader = fragmentShader;

	}


	const char* vertexShaderCode = R"(
			#version 330
			in vec3 position;
			void main() {
				gl_Position = vec4(position, 1.0);
			}
		)";

	if (!shader->addVertexShader(vertexShaderCode))
	{
		NLOGERROR(niceName, "Vertex shader compilation failed: " << shader->getLastError());
	}

	if (!shader->addFragmentShader(fShader))
	{
		NLOGERROR(niceName, "Fragment shader compilation failed: " << shader->getLastError());
		//LOGERROR(fShader);
	}

	if (!shader->link())
	{
		NLOGERROR(niceName, "Fragment shader link failed: " << shader->getLastError());
	}

	if (shader->getLastError().isEmpty())
	{
		NLOG(niceName, "Shader compiled and linked successfully");
	}
	else
	{
		shader.reset();
	}


}

void ShaderMedia::checkForHotReload()
{
	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();
	bool isFile = st == ShaderGLSLFile || st == ShaderToyFile;
	if (!isFile) return;

	File f = shaderFile->getFile();
	if (!f.existsAsFile()) return;
	if (f.getLastModificationTime() != lastModificationTime) shouldReloadShader = true;
}

void ShaderMedia::run()
{
	String id = shaderToyID->stringValue();
	String key = shaderToyKey->stringValue();
	String shaderStr = "";

	if (id.isEmpty()) NLOGWARNING(niceName, "ShaderToy ID is empty");
	if (key.isEmpty()) NLOGWARNING(niceName, "ShaderToy Key is empty");

	if (id.isNotEmpty() && key.isNotEmpty())
	{
		//get fragment shader from url

		URL url("https://www.shadertoy.com/api/v1/shaders/" + id + "?key=" + shaderToyKey->stringValue());
		LOG("Loading Shader at " << url.toString(true));

		if (!url.isWellFormed())
		{
			NLOGWARNING(niceName, "URL is alformed: " << url.toString(true));
			return;
		}

		String dataStr = url.readEntireTextStream(false);
		if (shaderStr.isEmpty())
		{
			NLOGWARNING(niceName, "Could not retrieve shader at " << url.toString(true));
		}

		var data = JSON::parse(dataStr);

		if (data.isObject())
		{
			if (data["Error"].isString())
			{
				NLOGWARNING(niceName, "ShaderToy Error: " << data["Error"].toString());
			}
			else
			{
				var shader = data["Shader"];
				if (shader.isObject())
				{
					var info = shader["info"];
					if (info.isObject())
					{
						NLOG(niceName, "Shader info : " << info["name"].toString() << "\n" << info["description"].toString());
					}
				}

				shaderStr = data["Shader"]["renderpass"][0]["code"].toString();
			}
		}
	}

	GlContextHolder::getInstance()->context.executeOnGLThread([&](OpenGLContext&) { loadFragmentShader(shaderStr); }, true);
}


//Timer
void ShaderCheckTimer::timerCallback()
{

	for (ShaderMedia* s : MediaManager::getInstance()->getItemsWithType<ShaderMedia>())
	{
		s->checkForHotReload();
	}
}
