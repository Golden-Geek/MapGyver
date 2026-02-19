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
	autoLoadShader(true),
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
	shaderType->addOption("Shader GLSL File", ShaderGLSLFile)->addOption("ShaderToy File", ShaderToyFile)->addOption("ShaderToy Online", ShaderToyURL)->addOption("ISF File", ShaderISFFile)->addOption("ISF Online", ShaderISFURL);

	shaderFile = addFileParameter("Fragment Shader", "Fragment Shader");
	shaderFile->setAutoReload(true);

	onlineShaderID = addStringParameter("Shader ID", "ID of the shader, either for ShaderToy or ISF. It's the last part of the URL when viewing it on the website", "dlVyDd", false);
	shaderToyKey = addStringParameter("ShaderToy Key", "Key of the shader toy. It's the last part of the URL when viewing it on the website", "Bd8jRr", false);
	keepOfflineCache = addBoolParameter("Keep Offline Cache", "Keep the offline cache of the shader, to reload if file is missing or if no internet for online shaders", true);

	shaderLoaded = addBoolParameter("Shader Loaded", "Shader Loaded", false);
	shaderLoaded->setControllableFeedbackOnly(true);


	fps = addIntParameter("FPS", "FPS", 60, 1, 1000);

	backgroundColor = addColorParameter("Background Color", "Background Color", Colours::black);
	speed = addFloatParameter("Speed", "Speed Multiplier", 1.0f);
	resetTime = addTrigger("Reset Time", "Reset Time to 0");
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

bool ShaderMedia::isUsingMedia(Media* m)
{
	if (!isBeingUsed->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
}

void ShaderMedia::onContainerTriggerTriggered(Trigger* t)
{
	if (t == resetTime)
	{
		currentTime = 0;
	}
}

void ShaderMedia::onContainerParameterChangedInternal(Parameter* p)
{
	Media::onContainerParameterChangedInternal(p);
	if (p == shaderType)
	{
		ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();
		shaderFile->setEnabled(st == ShaderGLSLFile || st == ShaderToyFile || st == ShaderISFFile);
		onlineShaderID->setEnabled(st == ShaderToyURL || st == ShaderISFURL);
		if (onlineShaderID->enabled && onlineShaderID->stringValue().isEmpty()) onlineShaderID->setValue(st == ShaderToyURL ? defaultShaderToyID : defaultISFID);
		shaderToyKey->setEnabled(st == ShaderToyURL);

		sourceMedias.userCanAddControllables = !(st == ShaderISFFile || st == ShaderISFURL);
	}

	if (p == shaderType || p == shaderFile || p == onlineShaderID || p == shaderToyKey)
	{
		if (!isLoadingShader)
		{
			shouldReloadShader = true;
			if (autoLoadShader)
			{
				shouldRedraw = true;
				forceRedraw = true;
			}
		}
	}

	if (p == shaderLoaded)
	{
		if (shaderLoaded->boolValue())
		{
			shouldGeneratePreviewImage = true;
			forceRedraw = true;
			shouldRedraw = true;
		}
	}

	if (p == isBeingUsed)
	{
		updateUsedMedias();
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

	timeAtLastFrame = Time::getMillisecondCounter();
}

void ShaderMedia::preRenderGLInternal()
{
	if (shouldReloadShader) reloadShader();
	if (fragmentShaderToLoad.isNotEmpty()) loadFragmentShader(fragmentShaderToLoad);
}

void ShaderMedia::renderGLInternal()
{
	//LOG("Render GL");

	double curT = Time::getMillisecondCounter();
	double  realDeltaTime = (curT - timeAtLastFrame) / 1000.0;
	currentTime += realDeltaTime * speed->floatValue();
	timeAtLastFrame = curT;

	double t = customTime >= 0 ? customTime * speed->floatValue() : currentTime;
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


	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	int texIndex = 0;
	int offset = 5;
	for (auto& tp : sourceMedias.controllables)
	{
		TargetParameter* p = dynamic_cast<TargetParameter*>(tp);
		if (Media* m = p->getTargetContainerAs<Media>())
		{
			glActiveTexture(GL_TEXTURE0 + texIndex + offset);
			glBindTexture(GL_TEXTURE_2D, m->getTextureID());

			String texName = textureUniformName.isEmpty() ? "" : (textureUniformName + String(texIndex));
			if (st == ShaderISFFile || st == ShaderISFURL) texName = p->niceName;

			if (texName.isNotEmpty())
			{
				shader->setUniform(texName.toStdString().c_str(), texIndex + offset);
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
		case 1:
			if (cp->type == Controllable::BOOL) shader->setUniform(p->niceName.toStdString().c_str(), (GLint)(int)val[0]);
			else shader->setUniform(p->niceName.toStdString().c_str(), (float)val);
			break;
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

	//LOG("load fragment shader");

	isLoadingShader = true;
	shaderLoaded->setValue(false);

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

	if (versionLine.isEmpty()) versionLine = "#version 330\n\n";


	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();
	bool isShaderToy = st == ShaderToyFile || st == ShaderToyURL;

	ShaderType newType = st;


	bool fragmentIsShaderToy = fragmentShader.contains("mainImage") && !fragmentShader.contains("main(") && !fragmentShader.contains("main (");
	if (fragmentIsShaderToy && isShaderToy != fragmentIsShaderToy) newType = ShaderToyFile;

	bool isISF = st == ShaderISFFile || st == ShaderISFURL;
	bool fragmentIsISF = fragmentShader.startsWith("/*");
	if (fragmentIsISF && isISF != fragmentIsISF)  newType = ShaderISFFile;

	if (!fragmentIsISF && !fragmentIsShaderToy) newType = ShaderGLSLFile;

	if (newType != st) shaderType->setValueWithData(newType);
	st = newType;

	String fullShader = lines.joinIntoString("\n");
	fullShader = insertShaderIncludes(fullShader);

	fullShader = parseUniforms(fullShader);

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
	case ShaderISFURL:
	{
		fShader += R"(
			int PASSINDEX;
			uniform vec2 RENDERSIZE;
			uniform float TIME;
			uniform float TIMEDELTA;
			uniform int FRAMEINDEX;
			uniform vec4 DATE;	
			uniform vec2 isf_FragNormCoord;

			vec4 IMG_PIXEL(sampler2D img, vec2 coord) {
				return texture(img, vec2(coord.x/RENDERSIZE.x,coord.y/RENDERSIZE.y) );
			}
			vec4 IMG_THIS_PIXEL(sampler2D img) {
				return texture(img, vec2(isf_FragNormCoord.x,isf_FragNormCoord.y) );
			}
			vec4 IMG_NORM_PIXEL(sampler2D img, vec2 coord) {
				return texture(img, vec2(coord.x,coord.y) );
			}
			vec4 IMG_THIS_NORM_PIXEL(sampler2D img) {
				return texture(img, vec2(isf_FragNormCoord.x,isf_FragNormCoord.y) );
			}

			)"
			+ fullShader
			;

		resolutionUniformName = "RENDERSIZE";
		timeUniformName = "TIME";
		timeDeltaUniformName = "TIMEDELTA";
		frameUniformName = "FRAMEINDEX";
		normCoordUniformName = "isf_FragNormCoord";

		useMouse4D = false;
		useResolution3D = false;

		//LOG(fShader);

	}
	break;
	}


	String vertexShaderCode = OpenGLHelpers::getGLSLVersionString() + "\n" + R"(
			in vec3 position;
			void main() {
				gl_Position = vec4(position, 1.0);
			}
		)";

	if (!shader->addVertexShader(vertexShaderCode.toRawUTF8()))
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
		shaderLoaded->setValue(true);
	}
	else
	{
		shader.reset();
	}


	shaderOfflineData = fragmentShaderToLoad;
	fragmentShaderToLoad = "";
	isLoadingShader = false;


	if (shaderLoaded->boolValue() && fragmentIsISF)
	{
		//update source medias with isf names
		while (sourceMedias.controllables.size() > isfTextureNames.size())
		{
			sourceMedias.controllables.removeLast();
		}

		for (int i = 0; i < sourceMedias.controllables.size(); i++)
		{
			TargetParameter* p = (TargetParameter*)sourceMedias.controllables[i];
			p->setNiceName(isfTextureNames[i]);
		}

		while (sourceMedias.controllables.size() < isfTextureNames.size())
		{
			String n = isfTextureNames[sourceMedias.controllables.size()];
			TargetParameter* p = sourceMedias.addTargetParameter(n, "Source Media for texture  " + n, MediaManager::getInstance());
			p->targetType = TargetParameter::CONTAINER;
			p->maxDefaultSearchLevel = 0;
			p->saveValueOnly = false;
		}

	}

	mediaNotifier.addMessage(new MediaEvent(MediaEvent::MEDIA_CONTENT_CHANGED, this));
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

String ShaderMedia::parseUniforms(const String& fragmentShader)
{
	String result = fragmentShader;

	detectedUniforms.clear();
	isfTextureNames.clear();

	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	if (st == ShaderISFFile || st == ShaderISFURL)
	{

		StringArray jsonData = RegexFunctions::getFirstMatch("\\/\\*([\\s\\S]+)\\*\\/", fragmentShader);
		if (jsonData.size() >= 2)
		{
			String jsonStr = jsonData[1];
			var json = JSON::parse(jsonStr);
			if (json.isObject())
			{

				String uniformsDeclaration = "";

				var inputs = json["INPUTS"];
				if (inputs.isArray())
				{
					for (int i = 0; i < inputs.size(); i++)
					{
						var input = inputs[i];
						String name = input["NAME"].toString();
						String type = input["TYPE"].toString();
						var minVal = input["MIN"];
						var maxVal = input["MAX"];
						var defaultVal = input["DEFAULT"];

						if (type == "event") continue;

						Controllable::Type controllableType = Controllable::Type::FLOAT;
						String uniformType;
						bool hasDefaultVal = true;
						bool addToUniforms = true;

						if (type == "bool")
						{
							controllableType = Controllable::Type::BOOL;
							uniformType = "bool";
						}
						if (type == "float")
						{
							controllableType = Controllable::Type::FLOAT;
							uniformType = "float";
						}
						else if (type == "int")
						{
							controllableType = Controllable::Type::INT;
							uniformType = "int";
						}
						else if (type == "point2D")
						{
							controllableType = Controllable::Type::POINT2D;
							uniformType = "vec2";
						}
						else if (type == "point3D")
						{
							controllableType = Controllable::Type::POINT3D;
							uniformType = "vec3";
						}
						else if (type == "color")
						{
							controllableType = Controllable::Type::COLOR;
							uniformType = "vec4";
						}
						else if (type == "image")
						{
							uniformType = "sampler2D";
							controllableType = Controllable::Type::CUSTOM;
							hasDefaultVal = false;
							addToUniforms = false;
							isfTextureNames.add(name);
						}


						if (uniformType.isNotEmpty())
						{
							if (addToUniforms) detectedUniforms.add({ name, controllableType, minVal, maxVal, defaultVal });

							String defaultValStr = "";

							if (!defaultVal.isArray())  defaultValStr = controllableType == Controllable::BOOL ? ((int)defaultVal ? "true" : "false") : defaultVal.toString();
							else
							{
								defaultValStr = uniformType + "(";
								for (int v = 0; v < defaultVal.size(); v++)
								{
									if (v > 0) defaultValStr += ",";
									defaultValStr += defaultVal[v].toString();
								}
								defaultValStr += ")";
							}

							uniformsDeclaration += "uniform " + uniformType + " " + name + (hasDefaultVal ? " = " + defaultValStr : "") + ";\n";
						}
					}
				}

				result = result.substring(jsonData[0].length());
				result = uniformsDeclaration + result;

			}
			else
			{
				NLOGERROR(niceName, "ISF Shader json parsing error");
			}
		}
		else
		{
			NLOGERROR(niceName, "ISF Shader needs to start with a comment block");
		}
	}
	else
	{


		String regex = "[ \\t]*uniform (\\w+) ([\\w\\d]+) ?=? ?([\\w(), \\d\\.]+)?;[ \\t]*(?:\\/\\/)?[ \\t]*([\\[,\\]\\d\\.]+)?:?([\\[,\\]\\d\\.]+)?";
		Array<StringArray> allMatches = RegexFunctions::getAllMatches(regex, fragmentShader);
		for (auto& m : allMatches)
		{
			String type = m[1];
			String name = m[2];
			String dVal = m[3];
			String minVal = m[4];
			String maxVal = m[5];


			Controllable::Type cType = Controllable::Type::CUSTOM;
			var uVal;
			var uMin;
			var uMax;

			if (type == "int")
			{
				cType = Controllable::INT;
				uVal = dVal.getIntValue();
				if (minVal.isNotEmpty()) uMin = minVal.getIntValue();
				if (maxVal.isNotEmpty()) uMax = maxVal.getIntValue();
			}
			else if (type == "float")
			{
				cType = Controllable::FLOAT;
				uVal = dVal.getFloatValue();
				if (minVal.isNotEmpty()) uMin = minVal.getFloatValue();
				if (maxVal.isNotEmpty()) uMax = maxVal.getFloatValue();
			}
			else if (type.startsWith("vec"))
			{
				String vecRegex = "\\w+\\(([\\d\\.]+)[, ]*([\\d\\.]+)[, ]*([\\d\\.]*)[, ]*([\\d\\.]*)\\)";
				Array<StringArray> vecMatches = RegexFunctions::getAllMatches(vecRegex, dVal);
				if (vecMatches.size() > 0)
				{

					if (vecMatches[0][1].isNotEmpty()) uVal.append((vecMatches[0][1]).getFloatValue());
					if (vecMatches[0][2].isNotEmpty()) uVal.append(vecMatches[0][2].getFloatValue());
					if (vecMatches[0][3].isNotEmpty()) uVal.append(vecMatches[0][3].getFloatValue());
					if (vecMatches[0][4].isNotEmpty()) uVal.append(vecMatches[0][4].getFloatValue());
				}
				else
				{
					LOG("No match for vec value");
				}

				if (type == "vec2" || type == "vec3")
				{
					String arrRegex = "\\[([\\d\\.]+)[, ]+([\\d\\.]+)(?:[, ]+)?([\\d\\.]+)?\\]";
					if (minVal.isNotEmpty())
					{
						Array<StringArray> minMatches = RegexFunctions::getAllMatches(arrRegex, minVal);
						if (minMatches.size() > 0)
						{
							if (minMatches[0][1].isNotEmpty()) uMin.append(minMatches[0][1].getFloatValue());
							if (minMatches[0][2].isNotEmpty()) uMin.append(minMatches[0][2].getFloatValue());
							if (minMatches[0][3].isNotEmpty()) uMin.append(minMatches[0][3].getFloatValue());
						}
					}

					if (maxVal.isNotEmpty())
					{
						Array<StringArray> maxMatches = RegexFunctions::getAllMatches(arrRegex, maxVal);
						if (maxMatches.size() > 0)
						{
							if (maxMatches[0][1].isNotEmpty()) uMax.append(maxMatches[0][1].getFloatValue());
							if (maxMatches[0][2].isNotEmpty()) uMax.append(maxMatches[0][2].getFloatValue());
							if (maxMatches[0][3].isNotEmpty()) uMax.append(maxMatches[0][3].getFloatValue());
						}
					}

					if (type == "vec2") cType = Controllable::POINT2D;
					else if (type == "vec3") cType = Controllable::POINT3D;
				}
				else if (type == "vec4") cType = Controllable::COLOR;
			}

			if (cType != Controllable::CUSTOM)
			{
				detectedUniforms.add({ name, cType, uMin, uMax, uVal });
			}
		}
	}

	NLOG(niceName, "Found " << detectedUniforms.size() << " parameters in shader");

	return result;
}

void ShaderMedia::showUniformControllableMenu(ControllableContainer* cc)
{
	PopupMenu m;
	m.addSectionHeader("Detected uniforms");
	for (auto& u : detectedUniforms)
	{
		if (u.name == "progression") continue;
		m.addItem(u.name, [this, u]() { addUniformControllable(u); });
	}
	m.addSeparator();
	m.addItem("Add all detected uniforms", [this, cc]()
		{
			for (auto& u : detectedUniforms)
			{
				if (u.name == "progression") continue;
				addUniformControllable(u);
			}
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
	var minVal = info.minVal.isVoid() ? var(0.f) : info.minVal;
	var maxVal = info.maxVal.isVoid() ? var(1.f) : info.maxVal;
	var defaultVal = info.defaultVal.isVoid() ? var(0.5f) : info.defaultVal;

	Controllable* c = nullptr;
	switch (info.type)
	{
	case Controllable::Type::BOOL: c = mediaParams.addBoolParameter(info.name, info.name, (int)defaultVal); break;
	case Controllable::Type::INT: c = mediaParams.addIntParameter(info.name, info.name, (float)defaultVal, (float)minVal, (float)maxVal); break;
	case Controllable::Type::FLOAT: c = mediaParams.addFloatParameter(info.name, info.name, (float)defaultVal, (float)minVal, (float)maxVal); break;
	case Controllable::Type::POINT2D:
		c = mediaParams.addPoint2DParameter(info.name, info.name);
		if (defaultVal.isArray()) ((Point2DParameter*)c)->setValue(defaultVal);
		if (minVal.isArray() && maxVal.isArray()) ((Point2DParameter*)c)->setBounds(minVal[0], minVal[1], maxVal[0], maxVal[1]);
		break;
	case Controllable::Type::POINT3D:
		c = mediaParams.addPoint3DParameter(info.name, info.name);
		if (defaultVal.isArray()) ((Point3DParameter*)c)->setValue(defaultVal);
		if (minVal.isArray() && maxVal.isArray()) ((Point3DParameter*)c)->setBounds(minVal[0], minVal[1], minVal[2], maxVal[0], maxVal[1], maxVal[2]);
		break;

	case Controllable::Type::COLOR:
		c = mediaParams.addColorParameter(info.name, info.name, Colours::red);
		if (defaultVal.isArray()) ((ColorParameter*)c)->setValue(defaultVal);
		break;
	}

	if (c != nullptr)
	{
		c->isRemovableByUser = true;
		c->isSavable = true;
		c->isCustomizableByUser = true;
		c->saveValueOnly = false;
	}
}

Point<float> ShaderMedia::getMediaShaderPosition(const MouseEvent& e, Rectangle<int> canvasRect)
{

	Point<float> p = (e.getPosition() - canvasRect.getPosition()).toFloat() / Point<float>(canvasRect.getWidth(), canvasRect.getHeight());
	p.y = 1.0f - p.y;
	return p;
}


void ShaderMedia::sendMouseDown(const MouseEvent& e, Rectangle<int> canvasRect)
{
	mouseClick->setValue(true);

}

void ShaderMedia::sendMouseUp(const MouseEvent& e, Rectangle<int> canvasRect)
{
	mouseClick->setValue(false);
}

void ShaderMedia::sendMouseDrag(const MouseEvent& e, Rectangle<int> canvasRect)
{
	mouseInputPos->setPoint(getMediaShaderPosition(e, canvasRect));

}

void ShaderMedia::sendMouseMove(const MouseEvent& e, Rectangle<int> canvasRect)
{
	mouseInputPos->setPoint(getMediaShaderPosition(e, canvasRect));

}

void ShaderMedia::sendMouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{

}

void ShaderMedia::sendKeyPressed(const KeyPress& key)
{
}

void ShaderMedia::run()
{
	String id = onlineShaderID->stringValue();
	String key = shaderToyKey->stringValue();
	String shaderStr = "";


	ShaderType st = shaderType->getValueDataAsEnum<ShaderType>();

	if (id.isEmpty())
	{
		NLOGWARNING(niceName, "Online Shader ID is empty, not loading");
		return;
	}

	if (st == ShaderToyURL && key.isEmpty())
	{
		NLOGWARNING(niceName, "ShaderToy Key is empty, not loading");
		return;
	}



	if (id.isNotEmpty() && key.isNotEmpty())
	{
		//get fragment shader from url

		String urlStr = "";
		if (st == ShaderToyURL) urlStr = "https://www.shadertoy.com/api/v1/shaders/" + id + "?key=" + shaderToyKey->stringValue();
		else if (st == ShaderISFURL) urlStr = "https://editor.isf.video/api/shaders/" + id;

		URL url(urlStr);

		LOG("Loading Shader at " << url.toString(true));

		if (!url.isWellFormed())
		{
			NLOGWARNING(niceName, "URL is malformed: " << url.toString(true));
			return;
		}
		juce::String customHeaders = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36\r\n"
			"Accept: application/json\r\n"
			"Accept-Language: en-US,en;q=0.9";
		std::unique_ptr<InputStream> stream = url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress).withExtraHeaders(customHeaders).withProgressCallback(
			[this](int, int) { return !threadShouldExit(); }));

		String dataStr = "";
		if (stream != nullptr) dataStr = stream->readEntireStreamAsString();
		else NLOGWARNING(niceName, "Could not retrieve shader at " << url.toString(true));

		if (dataStr.isEmpty())
		{
			NLOGWARNING(niceName, "No response when loading shader at " << url.toString(true));
		}


		var data = JSON::parse(dataStr);

		if (data.isObject())
		{
			if (st == ShaderToyURL)
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
			else if (st == ShaderISFURL)
			{
				shaderStr = data["rawFragmentSource"].toString();
			}
		}
		else
		{
			NLOGWARNING(niceName, "Could not parse JSON data from shader, response : " << data.toString());
		}
	}


	fragmentShaderToLoad = shaderStr.isNotEmpty() ? shaderStr : shaderOfflineData;
	if (autoLoadShader)
	{
		shouldRedraw = true;
		forceRedraw = true;
	}
}

String ShaderMedia::getMediaContentName() const
{
	File f = shaderFile->getFile();
	if (f.existsAsFile()) return f.getFileNameWithoutExtension();
	return Media::getMediaContentName();
}

var ShaderMedia::getJSONData(bool includeNonOverriden)
{
	var data = Media::getJSONData(includeNonOverriden);
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
		((TargetParameter*)c)->isRemovableByUser = true;
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
