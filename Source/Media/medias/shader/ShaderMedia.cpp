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
	shouldReloadShader(false),
	lastModificationTime(0),
	VBO(0),
	VAO(0)
{
	shaderFile = addFileParameter("Fragment Shader", "Fragment Shader");
	alwaysRedraw = true;
}

ShaderMedia::~ShaderMedia()
{
}

void ShaderMedia::onContainerParameterChangedInternal(Parameter* p)
{
	Media::onContainerParameterChangedInternal(p);
	if (p == shaderFile) shouldReloadShader = true;
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
	if (shouldReloadShader) loadShader();

	GenericScopedLock lock(shaderLock);
	if (shader == nullptr) return;

	Point<int> size = getMediaSize();

	shader->use();
	shader->setUniform("u_resolution", size.x, size.y);
	shader->setUniform("u_time", Time::getMillisecondCounter() / 1000.f);
	

	Init2DViewport(size.x, size.y);

	glClearColor(0, 0, 1, .2f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//Init2DMatrix(size.x, size.y);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
}

void ShaderMedia::loadShader()
{
	GenericScopedLock lock(shaderLock);
	//unload shader
	shader.reset();

	File f = shaderFile->getFile();
	lastModificationTime = f.getLastModificationTime();

	if (!f.existsAsFile()) return;
	{
		shader.reset(new OpenGLShaderProgram(GlContextHolder::getInstance()->context));
		String s = f.loadFileAsString();

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

		if (!shader->addFragmentShader(s))
		{
			NLOGERROR(niceName, "Fragment shader compilation failed: " << shader->getLastError());
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

	shouldReloadShader = false;
}


//Timer
void ShaderCheckTimer::timerCallback()
{

	for (ShaderMedia* s : MediaManager::getInstance()->getItemsWithType<ShaderMedia>())
	{
		File f = s->shaderFile->getFile();
		if (!f.existsAsFile()) continue;
		if (f.getLastModificationTime() != s->lastModificationTime) s->shouldReloadShader = true;
	}
}
