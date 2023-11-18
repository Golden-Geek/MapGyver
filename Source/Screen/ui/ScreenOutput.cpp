/*
  ==============================================================================

	ScreenOutput.cpp
	Created: 9 Nov 2023 8:51:23pm
	Author:  rosta

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"

using namespace juce::gl;

ScreenOutput::ScreenOutput(Screen* parent) :
	closestHandle(nullptr),
	manipSurface(nullptr)
{
	setOpaque(true);
	parentScreen = parent;

	openGLContext.setRenderer(this);
	openGLContext.attachTo(*this);
	openGLContext.setComponentPaintingEnabled(true);

	setWantsKeyboardFocus(true); // Permet à ce composant de recevoir le focus clavier
	addKeyListener(this);        // Ajoutez ce composant comme écouteur clavier

	stopLive();

}

ScreenOutput::~ScreenOutput()
{
	stopLive();
	openGLContext.detach();
}


void ScreenOutput::paint(Graphics& g)
{
	Colour col = isMouseButtonDown() ? Colours::yellow : Colours::cyan;

	if (manipSurface != nullptr)
	{
		Path surfacePath;
		surfacePath.addPath(manipSurface->quadPath);

		//apply transform that inverse the Y axis and scale to the component size
		surfacePath.applyTransform(AffineTransform::scale(getWidth(), getHeight()));
		surfacePath.applyTransform(AffineTransform::verticalFlip(getHeight()));

		g.setColour(col.withAlpha(.1f));
		g.fillPath(surfacePath);
		g.setColour(col.brighter(.3f));
		g.strokePath(surfacePath, PathStrokeType(2.0f));

		g.drawLine(Line<float>(getPointOnScreen(manipSurface->topLeft->getPoint()).toFloat(), getPointOnScreen(manipSurface->bottomRight->getPoint()).toFloat()), 2.0f);
		g.drawLine(Line<float>(getPointOnScreen(manipSurface->topRight->getPoint()).toFloat(), getPointOnScreen(manipSurface->bottomLeft->getPoint()).toFloat()), 2.0f);

	}
	else if (closestHandle != nullptr)
	{
		Point<int> mp = getMouseXYRelative();
		Point<int> hp = getLocalBounds().getRelativePoint(closestHandle->x, 1 - closestHandle->y);

		float angle = mp.getAngleToPoint(hp);

		Point<float> a1 = Point<float>(cosf(angle), sinf(angle)) * 20;
		Point<float> a2 = Point<float>(cosf(angle + MathConstants<float>::pi), sinf(angle + MathConstants<float>::pi)) * 20;

		Path p;
		p.startNewSubPath(mp.toFloat() + a1);
		p.lineTo(mp.toFloat() + a2);
		p.lineTo(hp.toFloat());
		p.closeSubPath();

		g.setColour(col.withAlpha(.5f));
		g.fillPath(p);
	}
}

void ScreenOutput::paintOverChildren(Graphics& g)
{

}

void ScreenOutput::mouseDown(const MouseEvent& e)
{
	if (manipSurface != nullptr)
	{
		posAtMouseDown = { manipSurface->topLeft->getPoint(), manipSurface->topRight->getPoint(), manipSurface->bottomLeft->getPoint(), manipSurface->bottomRight->getPoint() };

	}
	else if (closestHandle != nullptr)
	{
		posAtMouseDown = { closestHandle->getPoint() };
		overlapHandles.clear();
		if (e.mods.isShiftDown()) overlapHandles = parentScreen->getOverlapHandles(closestHandle);
	}
}

void ScreenOutput::mouseMove(const MouseEvent& e)
{
	if (e.mods.isAltDown())
	{
		closestHandle = nullptr;
		manipSurface = parentScreen->getSurfaceAt(getRelativeMousePos());
	}
	else if (!e.mods.isLeftButtonDown())
	{
		manipSurface = nullptr;
		closestHandle = parentScreen->getClosestHandle(getRelativeMousePos());
	}
	repaint();

}

void ScreenOutput::mouseDrag(const MouseEvent& e)
{
	Point<float> offsetRelative = (e.getOffsetFromDragStart().toFloat() * Point<float>(1, -1)) / Point<float>(getWidth(), getHeight());

	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = { manipSurface->topLeft, manipSurface->topRight, manipSurface->bottomLeft, manipSurface->bottomRight };


		for (int i = 0; i < handles.size(); i++) handles[i]->setPoint(posAtMouseDown[i] + offsetRelative);
	}
	else if (closestHandle != nullptr)
	{
		Point<float> tp = posAtMouseDown[0] + offsetRelative;
		if (e.mods.isCommandDown())
		{
			Point2DParameter* th = parentScreen->getSnapHandle(tp, closestHandle);
			if (th != nullptr) tp = th->getPoint();
		}

		closestHandle->setPoint(tp);
		for (auto& h : overlapHandles) h->setPoint(tp);

	}
	repaint();
}

void ScreenOutput::mouseUp(const MouseEvent& e)
{
	if (manipSurface != nullptr)
	{
		Array<Point2DParameter*> handles = { manipSurface->topLeft, manipSurface->topRight, manipSurface->bottomLeft, manipSurface->bottomRight };
		Array<UndoableAction*> actions;
		for (int i = 0; i < handles.size(); i++) actions.add(handles[i]->setUndoablePoint(posAtMouseDown[i], handles[i]->getPoint(), true));
		UndoMaster::getInstance()->performActions("Move surface", actions);

	}
	else if (closestHandle != nullptr)
	{

		if (overlapHandles.size() > 0)
		{
			Array<UndoableAction*> actions;
			actions.add(closestHandle->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint(), true));
			for (auto& h : overlapHandles) actions.add(h->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint(), true));
			UndoMaster::getInstance()->performActions("Move handles", actions);
		}
		else closestHandle->setUndoablePoint(posAtMouseDown[0], closestHandle->getPoint());
		overlapHandles.clear();
	}
	repaint();
}

void ScreenOutput::mouseExit(const MouseEvent& e)
{
	closestHandle = nullptr;
}

Point<float> ScreenOutput::getRelativeMousePos()
{
	return getRelativeScreenPos(getMouseXYRelative());
}

Point<float> ScreenOutput::getRelativeScreenPos(Point<int> screenPos)
{
	Point<float> p = screenPos.toFloat();
	return Point<float>(p.x / getWidth(), 1 - (p.y / getHeight()));
}

Point<int> ScreenOutput::getPointOnScreen(Point<float> pos)
{
	return Point<float>(pos.x * getWidth(), (1 - pos.y) * getHeight()).toInt();
}

void ScreenOutput::goLive(int screenId)
{
	Displays ds = Desktop::getInstance().getDisplays();
	if (isLive)
	{
		return;
	}
	if (screenId >= ds.displays.size())
	{
		LOGERROR("selected display is not available");
		stopLive();
		return;
	}

	if (parentScreen->enabled != nullptr)
	{
		parentScreen->enabled->setValue(true);
	}

	isLive = true;
	Displays::Display d = ds.displays[screenId];
	openGLContext.setContinuousRepainting(true);

	//previousParent = getParentComponent();
	Rectangle<int> a = d.totalArea;
	a.setWidth(a.getWidth());
	a.setHeight(a.getHeight());
	addToDesktop(0);
	setVisible(true);
	setBounds(a);
	setAlwaysOnTop(true);
	repaint();
}

void ScreenOutput::stopLive()
{
	if (parentScreen->enabled != nullptr)
	{
		//parentScreen->enabled->setValue(false);
	}
	if (!isLive)
	{
		return;
	}
	setSize(1, 1);
	openGLContext.triggerRepaint();
	openGLContext.setContinuousRepainting(false);
	isLive = false;
	setAlwaysOnTop(false);
	//setVisible(false);
	//removeFromDesktop();
	//setAlwaysOnTop(false);
	//previousParent->addAndMakeVisible(this);
	//previousParent->resized();
}

void ScreenOutput::newOpenGLContextCreated()
{
	// Set up your OpenGL state here
	gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
	glDisable(GL_DEBUG_OUTPUT);

	createAndLoadShaders();
}

void ScreenOutput::renderOpenGL()
{
	// Définir la vue OpenGL en fonction de la taille du composant
	if (!isLive)
	{
		return;
	}

	glViewport(0, 0, getWidth(), getHeight());

	if (shader != nullptr)
	{
		shader->use();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (parentScreen != nullptr)
		{
			Point<float> tl, tr, bl, br;
			for (int i = 0; i < parentScreen->surfaces.items.size(); i++)
			{
				Surface* s = parentScreen->surfaces.items[i];
				if (s->enabled->boolValue())
				{
					Media* m = dynamic_cast<Media*>(s->media->targetContainer.get());
					std::shared_ptr<OpenGLTexture> tex = nullptr;

					//myTexture.bind();
					if (m != nullptr)
					{
						if (!textures.contains(m))
						{
							tex = std::make_shared<OpenGLTexture>();
							tex->loadImage(m->image);
							texturesVersions.set(m, m->imageVersion);
							textures.set(m, tex);
						}
						tex = textures.getReference(m);
						unsigned int vers = texturesVersions.getReference(m);
						if (m->imageVersion != vers)
						{
							tex->loadImage(m->image);
							texturesVersions.set(m, m->imageVersion);
						}
						tex->bind();
					}

					// vertices start

					// Create a vertex buffer object
					GLuint vbo;
					glGenBuffers(1, &vbo);
					glBindBuffer(GL_ARRAY_BUFFER, vbo);

					GLint posAttrib = glGetAttribLocation(shader->getProgramID(), "position");
					glEnableVertexAttribArray(posAttrib);
					glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

					GLint surfacePosAttrib = glGetAttribLocation(shader->getProgramID(), "surfacePosition");
					glEnableVertexAttribArray(surfacePosAttrib);
					glVertexAttribPointer(surfacePosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(float)));

					GLint texAttrib = glGetAttribLocation(shader->getProgramID(), "texcoord");
					glEnableVertexAttribArray(texAttrib);
					glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));

					GLuint borderSoftLocation = glGetUniformLocation(shader->getProgramID(), "borderSoft");
					glUniform4f(borderSoftLocation, s->softEdgeTop->floatValue(), s->softEdgeRight->floatValue(), s->softEdgeBottom->floatValue(), s->softEdgeLeft->floatValue());

					GLuint ebo;
					glGenBuffers(1, &ebo);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

					s->verticesLock.enter();
					glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * s->vertices.size(), s->vertices.getRawDataPointer(), GL_STATIC_DRAW);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->verticesElements.size()*sizeof(GLuint), s->verticesElements.getRawDataPointer(), GL_STATIC_DRAW);
					s->verticesLock.exit();

					glDrawElements(GL_TRIANGLES, s->verticesElements.size(), GL_UNSIGNED_INT, 0);

					if (tex != nullptr)
					{
						tex->unbind();
					}

				}

			}
		}
		glDisable(GL_BLEND);
	}
}

void ScreenOutput::openGLContextClosing()
{
	glEnable(GL_BLEND);
	textures.clear();
	glDisable(GL_BLEND);
	shader = nullptr;
}

void ScreenOutput::userTriedToCloseWindow()
{
	stopLive();
}

void ScreenOutput::createAndLoadShaders()
{
	//String vertexShader, fragmentShader;

	const char* vertexShaderCode =
		"in vec2 position;\n"
		"in vec2 surfacePosition;\n"
		"in vec3 texcoord;\n"
		"out vec3 Texcoord;\n"
		"out vec2 SurfacePosition;\n"
		"void main()\n"
		"{\n"
		"    Texcoord = texcoord;\n"
		"    SurfacePosition[0] = (surfacePosition[0]+1.0f)/2.0f;\n"
		"    SurfacePosition[1] = (surfacePosition[1]+1.0f)/2.0f;\n"
		"    gl_Position = vec4(position,0,1);\n"
		"}\n";

	const char* fragmentShaderCode =
		"in vec3 Texcoord;\n"
		"in vec2 SurfacePosition;\n"
		"out vec4 outColor;\n"
		"uniform sampler2D tex;\n"
		"uniform vec4 borderSoft;\n"
		"float map(float value, float min1, float max1, float min2, float max2) {\n"
		"return min2 + ((max2-min2)*(value-min1)/(max1-min1)); };"
		"void main()\n"
		"{\n"
		"    outColor = textureProj(tex, Texcoord);\n"
		"   float alpha = 1.0f;\n"
		"   if (SurfacePosition[1] > 1-borderSoft[0])    {alpha *= map(SurfacePosition[1],1.0f,1-borderSoft[0],0.0f,1.0f);}\n" // top
		"   if (SurfacePosition[0] > 1.0f-borderSoft[1]) {alpha *= map(SurfacePosition[0], 1.0f, 1 - borderSoft[1], 0.0f, 1.0f); }\n" // right
		"   if (SurfacePosition[1] < borderSoft[2])      {alpha *= map(SurfacePosition[1],0.0f,borderSoft[2],0.0f,1.0f);}\n" // bottom
		"   if (SurfacePosition[0] < borderSoft[3])      {alpha *= map(SurfacePosition[0],0.0f,borderSoft[3],0.0f,1.0f);}\n" // left
		"    outColor[3] = alpha;\n"
		//"    outColor = vec4(0.5f,0.5f,0.5f,0.5f);   "
		"}\n";

	shader.reset(new OpenGLShaderProgram(openGLContext));
	shader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(vertexShaderCode));
	shader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fragmentShaderCode));
	shader->link();

}

bool ScreenOutput::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (key.isKeyCode(key.escapeKey))
	{
		stopLive();
		return true;
	}
	return false;
}
