/*
  ==============================================================================

	MediaShader.cpp
	Created: 22 Nov 2023 3:39:16pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "ShaderMedia.h"

ShaderMedia::ShaderMedia(var params) :
	Media(getTypeString(), params, true),
	Thread("ShaderToy Loader"),
	shouldReloadShader(false),
	isLoadingShader(false),
	lastModificationTime(0),
	currentFrame(0),
	lastFrameTime(0),
	VBO(0),
	VAO(0),
	useMouse4D(false),
	sourceMedias("Source Medias")
{
	autoClearFrameBufferOnRender = false;

	shaderType = addEnumParameter("Shader Type", "Type Shader to load");
	shaderType->addOption("Shader GLSL File", ShaderGLSLFile)->addOption("ShaderToy File", ShaderToyFile)->addOption("ShaderToy Online", ShaderToyURL);

	shaderFile = addFileParameter("Fragment Shader", "Fragment Shader");
	shaderFile->setAutoReload(true);

	shaderToyID = addStringParameter("ShaderToy ID", "ID of the shader toy. It's the last part of the URL when viewing it on the website", "tsXBzS", false);
	shaderToyKey = addStringParameter("ShaderToy Key", "Key of the shader toy. It's the last part of the URL when viewing it on the website", "Bd8jRr", false);
	keepOfflineCache = addBoolParameter("Keep Offline Cache", "Keep the offline cache of the shader, to reload if file is missing or if no internet for online shaders", true);


	fps = addIntParameter("FPS", "FPS", 60, 1, 1000);

	backgroundColor = addColorParameter("Background Color", "Background Color", Colours::black);
	mouseClick = addBoolParameter("Mouse Click", "Simulates mouse click, for shader toy", false);
	mouseInputPos = addPoint2DParameter("Mouse Input Pos", "Mouse Input Pos");
	mouseInputPos->setBounds(0, 0, 1, 1);

	mediaParams.userCanAddControllables = true;
	mediaParams.customUserCreateControllableFunc = std::bind(&ShaderMedia::showUniformControllableMenu, this, std::placeholders::_1);

	sourceMedias.userCanAddControllables = true;
	sourceMedias.customUserCreateControllableFunc = [&](ControllableContainer* cc)
		{
			TargetParameter* p = cc->addTargetParameter("Source Media", "Source Media", MediaManager::getInstance());
			p->isRemovableByUser = true;
			p->targetType = TargetParameter::CONTAINER;
			p->maxDefaultSearchLevel = 0;
			p->saveValueOnly = false;
		};
	addChildControllableContainer(&sourceMedias);


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
		shaderFile->setEnabled(st == ShaderGLSLFile || st == ShaderToyFile || st == ShaderISFFile);
		shaderToyID->setEnabled(st == ShaderToyURL);
		shaderToyKey->setEnabled(st == ShaderToyURL);
	}

	if (p == shaderType || p == shaderFile || p == shaderToyID || p == shaderToyKey)
	{
		if (!isLoadingShader) shouldReloadShader = true;
	}
}

void ShaderMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	Media::onControllableFeedbackUpdateInternal(cc, c);
	if (cc == &sourceMedias)
	{
		int i = 0;
		for (auto& tp : sourceMedias.controllables)
		{
			TargetParameter* p = dynamic_cast<TargetParameter*>(tp);
			Media* m = p->getTargetContainerAs<Media>();
			if (m != nullptr) registerUseMedia(i, m);
			i++;
		}
		unregisterUseMedia(i); //if there are less sources than before, force unregister
	}
}

void ShaderMedia::initGLInternal()
{
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
}

void ShaderMedia::preRenderGLInternal()
{
	if (shouldReloadShader) reloadShader();
	if (fragmentShaderToLoad.isNotEmpty()) loadFragmentShader(fragmentShaderToLoad);
}

void ShaderMedia::renderGLInternal()
{
	//LOG("Render GL");
	double t = customTime >= 0 ? customTime : Time::getMillisecondCounter() / 1000.0;
	if (customTime >= 0) firstFrameTime = 0;
	else if (firstFrameTime == 0) firstFrameTime = t;
	t = t - firstFrameTime;
	float delta = t - lastFrameTime;

	//float frameTime = 1.0f / fps->floatValue();
	//if (customTime < 0 && t < lastFrameTime + frameTime) return;

	lastFrameTime = t;

	if (isLoadingShader) return;

	//GenericScopedLock lock(shaderLock);
	if (shader == nullptr) return;

	Point<int> size = getMediaSize();

	shader->use();


	Point<float> mousePos = mouseInputPos->getPoint() * Point<float>(size.x, size.y);

	//Set uniforms
	if (useResolution3D) shader->setUniform(resolutionUniformName.toStdString().c_str(), size.x, size.y, 0.f);
	else if (resolutionUniformName.isNotEmpty())	shader->setUniform(resolutionUniformName.toStdString().c_str(), size.x, size.y);
	if (timeUniformName.isNotEmpty()) shader->setUniform(timeUniformName.toStdString().c_str(), (float)t);
	if (timeDeltaUniformName.isNotEmpty()) shader->setUniform(timeDeltaUniformName.toStdString().c_str(), delta);
	if (frameUniformName.isNotEmpty()) shader->setUniform(frameUniformName.toStdString().c_str(), currentFrame);


	if (mouseUniformName.isNotEmpty())
	{
		if (useMouse4D) shader->setUniform(mouseUniformName.toStdString().c_str(), mousePos.x, mousePos.y, mouseClick->floatValue(), 0.f);
		else if (mouseUniformName.isNotEmpty()) shader->setUniform(mouseUniformName.toStdString().c_str(), mousePos.x, mousePos.y);
	}


	int texIndex = 0;
	int offset = 5;
	for (auto& tp : sourceMedias.controllables)
	{
		TargetParameter* p = dynamic_cast<TargetParameter*>(tp);
		if (Media* m = p->getTargetContainerAs<Media>())
		{
			if (textureUniformName.isNotEmpty())
			{
				glActiveTexture(GL_TEXTURE0 + texIndex + offset);
				glBindTexture(GL_TEXTURE_2D, m->getTextureID());
				shader->setUniform((textureUniformName + String(texIndex)).toStdString().c_str(), texIndex + offset);
				texIndex++;
			}
		}
	};

	for (auto& cp : mediaParams.controllables)
	{
		if (!cp->enabled) continue;
		Parameter* p = dynamic_cast<Parameter*>(cp);
		shader->getUniformIDFromName(p->niceName.toStdString().c_str());
		var val = p->getValue();
		switch (val.size())
		{
		case 0: shader->setUniform(p->niceName.toStdString().c_str(), (float)val); break;
		case 1: shader->setUniform(p->niceName.toStdString().c_str(), (float)val[0]); break;
		case 2: shader->setUniform(p->niceName.toStdString().c_str(), (float)val[0], (float)val[1]); break;
		case 3: shader->setUniform(p->niceName.toStdString().c_str(), (float)val[0], (float)val[1], (float)val[2]); break;
		case 4: shader->setUniform(p->niceName.toStdString().c_str(), (float)val[0], (float)val[1], (float)val[2], (float)val[3]); break;
		}
	}

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

	for (int i = 0; i < texIndex; i++)
	{
		glActiveTexture(GL_TEXTURE0 + texIndex + offset);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);

	currentFrame++;
}

void ShaderMedia::reloadShader()
{
	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	bool isFile = st == ShaderGLSLFile || st == ShaderToyFile || st == ShaderISFFile;
	if (isFile)
	{
		File sf = shaderFile->getFile();
		String s = "";
		if (sf.existsAsFile())
		{
			s = shaderFile->getFile().loadFileAsString();
			lastModificationTime = sf.getLastModificationTime();
		}
		else if (keepOfflineCache->boolValue())
		{
			s = shaderOfflineData;
		}

		fragmentShaderToLoad = s;
	}
	else
	{
		startThread();
	}

	shouldReloadShader = false;
}

void ShaderMedia::loadFragmentShader(const String& fragmentShader)
{
	//GenericScopedLock lock(shader);

	LOG("load fragment shader");

	isLoadingShader = true;

	//unload shader
	shader.reset();

	if (fragmentShader.isEmpty())
	{
		isLoadingShader = false;
		return;

	}

	//retrieve and remove #version directive from fragment shader
	StringArray lines;
	lines.addLines(fragmentShader);
	String versionLine = "";
	for (int i = 0; i < lines.size(); i++)
	{
		String l = lines[i].trimCharactersAtStart(" \t");
		if (l.startsWith("#version"))
		{
			versionLine = l;
			lines.remove(i);
			break;
		}
	}

	String fullShader = lines.joinIntoString("\n");
	fullShader = insertShaderIncludes(fullShader);

	retrieveUniforms(fullShader);

	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();
	bool isShaderToy = st == ShaderToyFile || st == ShaderToyURL;
	bool fragmentIsShaderToy = fullShader.contains("mainImage");
	if (isShaderToy != fragmentIsShaderToy) shaderType->setValueWithData(fragmentIsShaderToy ? ShaderToyFile : ShaderGLSLFile);

	shader.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));

	String fShader = versionLine;

	st = shaderType->getValueDataAsEnum<ShaderType>();

	switch (st)
	{
	case ShaderToyFile:
	case ShaderToyURL:
	{
		fShader += R"(
			#ifdef GL_ES
			precision mediump float;
			#endif

			uniform vec3      iResolution;           // viewport resolution (in pixels)
			uniform float     iTime;                 // shader playback time (in seconds)
			uniform float     iTimeDelta;            // render time (in seconds)
			uniform float     iFrameRate;            // shader frame rate
			uniform int       iFrame;                // shader playback frame
			uniform float     iChannelTime[4];       // channel playback time (in seconds)
			uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
			uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
			uniform vec4      iDate;                 // (year, month, day, time in seconds)
			uniform sampler2D iChannel0;             // input channel. XX = 2D/Cube
			uniform sampler2D iChannel1;             // input channel. XX = 2D/Cube
			uniform sampler2D iChannel2;             // input channel. XX = 2D/Cube
			uniform sampler2D iChannel3;             // input channel. XX = 2D/Cube
			)"
			+ fullShader
			+ R"(

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
		timeDeltaUniformName = "iTimeDelta";
		normCoordUniformName = "";
		textureUniformName = "iChannel";

		useMouse4D = true;
		useResolution3D = true;

	}
	break;

	case ShaderGLSLFile:
	{
		resolutionUniformName = "u_resolution";
		timeUniformName = "u_time";
		mouseUniformName = "u_mouse";
		frameUniformName = "";
		timeDeltaUniformName = "";
		normCoordUniformName = "";

		useMouse4D = false;
		useResolution3D = false;

		fShader += fullShader;

	}
	break;

	case ShaderISFFile:
	{
		resolutionUniformName = "RENDERSIZE";
		timeUniformName = "TIME";
		timeDeltaUniformName = "TIMEDELTA";
		frameUniformName = "FRAMEINDEX";
		normCoordUniformName = "isf_FragNormCoord";

		useMouse4D = false;
		useResolution3D = false;
		fShader += fullShader;
	}
	break;
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

	shaderOfflineData = fragmentShaderToLoad;
	fragmentShaderToLoad = "";
	isLoadingShader = false;

}

String ShaderMedia::insertShaderIncludes(const String& fragmentShader)
{
	StringArray lines;
	lines.addLines(fragmentShader);

	String newShader = "";
	for (auto& l : lines)
	{
		l = l.trimCharactersAtStart(" \t");
		if (l.startsWith("#include"))
		{
			StringArray parts;
			parts.addTokens(l, true);
			if (parts.size() > 1)
			{
				String includeFile = parts[1];
				includeFile = includeFile.removeCharacters("\"");
				File f = shaderFile->getFile().getParentDirectory().getChildFile(includeFile);
				if (f.existsAsFile())
				{
					String includeShader = f.loadFileAsString();
					newShader += insertShaderIncludes(includeShader);
				}
				else
				{
					NLOGERROR(niceName, "Include file not found: " << f.getFullPathName());
				}
			}
			else
			{
				NLOGWARNING(niceName, "Include line is malformed: " << l);
			}
		}
		else
		{
			newShader += l + "\n";
		}
	}

	return newShader;
}

void ShaderMedia::retrieveUniforms(const String& fragmentShader)
{
	StringArray lines;
	lines.addLines(fragmentShader);

	detectedUniforms.clear();

	for (auto& l : lines)
	{
		l = l.trimCharactersAtStart(" \t");
		if (l.startsWith("uniform"))
		{
			StringArray parts;
			parts.addTokens(l, true);
			if (parts.size() > 2)
			{
				String type = parts[1];
				String name = parts[2];

				//add all uniforms to detected uniforms with their corresponding Controllable Type
				if (type == "int") detectedUniforms.add({ name, Controllable::Type::INT });
				else if (type == "float") detectedUniforms.add({ name, Controllable::Type::FLOAT });
				else if (type == "vec2") detectedUniforms.add({ name, Controllable::Type::POINT2D });
				else if (type == "vec3") detectedUniforms.add({ name, Controllable::Type::POINT3D });
				else if (type == "vec4") detectedUniforms.add({ name, Controllable::Type::COLOR });
			}
		}
	}
}

void ShaderMedia::showUniformControllableMenu(ControllableContainer* cc)
{
	PopupMenu m;
	m.addSectionHeader("Detected uniforms");
	for (auto& u : detectedUniforms) m.addItem(u.name, [this, u]() { addUniformControllable(u); });
	m.addSeparator();
	m.addItem("Add all detected uniforms", [this, cc]()
		{
			for (auto& u : detectedUniforms) addUniformControllable(u);
		});
	m.addSeparator();
	m.addSectionHeader("Custom uniforms");
	m.addItem("Add Int", [this]() { addUniformControllable({ "myInt", Controllable::Type::FLOAT }); });
	m.addItem("Add Float", [this]() { addUniformControllable({ "myFloat", Controllable::Type::FLOAT }); });
	m.addItem("Add Point2D", [this]() { addUniformControllable({ "myPoint2D", Controllable::Type::POINT2D }); });
	m.addItem("Add Point3D", [this]() { addUniformControllable({ "myPoint3D", Controllable::Type::POINT3D }); });
	m.addItem("Add Color", [this]() { addUniformControllable({ "myColor", Controllable::Type::COLOR }); });

	m.showMenuAsync(PopupMenu::Options());
}

void ShaderMedia::addUniformControllable(UniformInfo info)
{
	Controllable* c = nullptr;
	switch (info.type)
	{
	case Controllable::Type::INT: c = mediaParams.addIntParameter(info.name, info.name, .5, 0, 1); break;
	case Controllable::Type::FLOAT: c = mediaParams.addFloatParameter(info.name, info.name, .5, 0, 1); break;
	case Controllable::Type::POINT2D: c = mediaParams.addPoint2DParameter(info.name, info.name); break;
	case Controllable::Type::POINT3D: c = mediaParams.addPoint3DParameter(info.name, info.name); break;
	case Controllable::Type::COLOR: c = mediaParams.addColorParameter(info.name, info.name, Colours::red); break;
	}

	if (c != nullptr)
	{
		c->isRemovableByUser = true;
		c->isSavable = true;
		c->isCustomizableByUser = true;
		c->saveValueOnly = false;
	}
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

		std::unique_ptr<InputStream> stream = url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress).withProgressCallback(
			[this](int, int) { return !threadShouldExit(); }));

		String dataStr = "";
		if (stream != nullptr) dataStr = stream->readEntireStreamAsString();
		else NLOGWARNING(niceName, "Could not retrieve shader at " << url.toString(true));

		if (dataStr.isEmpty())
		{
			NLOGWARNING(niceName, "No responde when loading shader at " << url.toString(true));
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

				if (data["Shader"]["renderpass"].size() > 1)
				{
					LOGWARNING("ShaderToy has more than one render pass, not supported right now");
				}
				else
				{
					shaderStr = data["Shader"]["renderpass"][0]["code"].toString();
				}
			}
		}
	}


	fragmentShaderToLoad = shaderStr.isNotEmpty() ? shaderStr : shaderOfflineData;
}

var ShaderMedia::getJSONData()
{
	var data = Media::getJSONData();
	data.getDynamicObject()->setProperty("shaderCache", shaderOfflineData);
	data.getDynamicObject()->setProperty(sourceMedias.shortName, sourceMedias.getJSONData());
	return data;
}

void ShaderMedia::loadJSONDataItemInternal(var data)
{
	Media::loadJSONDataItemInternal(data);
	for (auto& c : sourceMedias.controllables)
	{
		((TargetParameter*)c)->setRootContainer(MediaManager::getInstance());
		((TargetParameter*)c)->targetType = TargetParameter::CONTAINER;
		((TargetParameter*)c)->maxDefaultSearchLevel = 0;
		((TargetParameter*)c)->saveValueOnly = false;
		((TargetParameter*)c)->isRemovableByUser = false;
	}

	for (auto& c : mediaParams.controllables)
	{
		c->isRemovableByUser = true;
		c->isSavable = true;
		c->isCustomizableByUser = true;
		c->saveValueOnly = false;
	}

	if (data.getDynamicObject()->hasProperty("shaderCache")) shaderOfflineData = data.getDynamicObject()->getProperty("shaderCache");
}
