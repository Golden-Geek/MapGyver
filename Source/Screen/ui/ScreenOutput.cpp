/*
  ==============================================================================

	ScreenOutput.cpp
	Created: 9 Nov 2023 8:51:23pm
	Author:  rosta

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"
#include "ScreenOutput.h"

juce_ImplementSingleton(ScreenOutputWatcher)

using namespace juce::gl;

ScreenOutput::ScreenOutput(Screen* screen) :
	InspectableContentComponent(screen),
	OpenGLSharedRenderer(this),
	isLive(false),
	screen(screen),
	timeAtRender(0)
{
	autoDrawContourWhenSelected = false;

	setWantsKeyboardFocus(true);
	setInterceptsMouseClicks(true, true);
}

ScreenOutput::~ScreenOutput()
{
	removeFromDesktop();
}


void ScreenOutput::update()
{
	bool shouldShow = !inspectable.wasObjectDeleted() && screen->enabled->boolValue();

	Displays ds = Desktop::getInstance().getDisplays();
	if (screen->screenID->intValue() >= ds.displays.size())
	{
		LOGWARNING("Display #" << screen->screenID->intValue() << " is not available(" + ds.displays.size() << " screens connected)");
		shouldShow = false;
	}

	bool prevIsLive = isLive;
	isLive = shouldShow;

	if (shouldShow)
	{
		Displays::Display d = ds.displays[screen->screenID->intValue()];

		Rectangle<int> a = d.totalArea;
		if (screen->positionCC.enabled->boolValue())
		{
			a.setX(a.getX() + screen->screenX->intValue());
			a.setY(a.getY() + screen->screenY->intValue());
			a.setWidth(screen->screenWidth->intValue());
			a.setHeight(screen->screenHeight->intValue());
		}
		else
		{
			a.setWidth(a.getWidth());
			a.setHeight(a.getHeight());
		}

		setBounds(a);

		if (!prevIsLive)
		{
			setAlwaysOnTop(true);
			setVisible(true);
			setOpaque(true);
			addKeyListener(this);
			addToDesktop(ComponentPeer::StyleFlags::windowRequiresSynchronousCoreGraphicsRendering);
		}

		repaint();

	}
	else
	{
		if (prevIsLive)
		{
			removeFromDesktop();
			setAlwaysOnTop(false);
			removeKeyListener(this);
		}

	}

	setVisible(shouldShow);
}

void ScreenOutput::newOpenGLContextCreated()
{
	// Set up your OpenGL state here
	gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
	glDisable(GL_DEBUG_OUTPUT);
}

void ScreenOutput::renderOpenGL()
{
	if (inspectable.wasObjectDeleted()) return;
	if (screen->isClearing) return;

	// Définir la vue OpenGL en fonction de la taille du composant
	if (!isLive)
	{
		return;
	}

	//context.makeActive();

	Init2DViewport(getWidth(), getHeight());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, screen->renderer->frameBuffer.getTextureID());
	glGetError();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Draw2DTexRect(0, 0, getWidth(), getHeight());

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGetError();

}

void ScreenOutput::openGLContextClosing()
{
}

void ScreenOutput::userTriedToCloseWindow()
{
	if (inspectable.wasObjectDeleted()) return;
	screen->enabled->setValue(false);
}

void ScreenOutput::newMessage(const Parameter::ParameterEvent& e)
{
	update();
}


bool ScreenOutput::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (inspectable.wasObjectDeleted()) return false;

	if (key.isKeyCode(key.escapeKey))
	{
		screen->enabled->setValue(false);
		return true;
	}

	return false;
}


ScreenOutputWatcher::ScreenOutputWatcher()
{
	ScreenManager::getInstance()->addAsyncManagerListener(this);
	ScreenManager::getInstance()->addAsyncContainerListener(this);
	Engine::mainEngine->addEngineListener(this);


}

ScreenOutputWatcher::~ScreenOutputWatcher()
{
	ScreenManager::getInstance()->removeAsyncManagerListener(this);
	ScreenManager::getInstance()->removeAsyncContainerListener(this);
	Engine::mainEngine->removeEngineListener(this);
	outputs.clear();
}

void ScreenOutputWatcher::updateOutput(Screen* s, bool forceRemove)
{
	ScreenOutput* o = getOutputForScreen(s);
	bool shouldShow = !forceRemove && !s->isClearing && s->enabled->boolValue() && s->outputType->getValueDataAsEnum<Screen::OutputType>() == Screen::OutputType::DISPLAY;
	if (o == nullptr)
	{
		if (shouldShow) o = outputs.add(new ScreenOutput(s));
	}
	else
	{
		if (!shouldShow)
		{
			outputs.removeObject(o);
			o = nullptr;
		}
	}

	if (o != nullptr) o->update();
}

ScreenOutput* ScreenOutputWatcher::getOutputForScreen(Screen* s)
{
	for (auto& o : outputs)
	{
		if (o->screen == s) return o;
	}
	return nullptr;
}


void ScreenOutputWatcher::newMessage(const ScreenManager::ManagerEvent& e)
{
	if (Engine::mainEngine->isLoadingFile) return;
	switch (e.type)
	{
	case ScreenManager::ManagerEvent::ITEM_ADDED:
		updateOutput(e.getItem());
		break;

	case ScreenManager::ManagerEvent::ITEMS_ADDED:
		for (auto& s : e.getItems()) updateOutput(s);
		break;


	case ScreenManager::ManagerEvent::ITEM_REMOVED:
		updateOutput(e.getItem(), true);
		break;

	case ScreenManager::ManagerEvent::ITEMS_REMOVED:
		for (auto& s : e.getItems()) updateOutput(s, true);
		break;
	}
}

void ScreenOutputWatcher::newMessage(const ContainerAsyncEvent& e)
{
	if (Engine::mainEngine->isLoadingFile) return;

	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
	{
		if (Screen* s = ControllableUtil::findParentAs<Screen>(e.targetControllable, 2))
		{
			if (e.targetControllable == s->enabled || e.targetControllable == s->outputType || e.targetControllable == s->screenID ||
				e.targetControllable->parentContainer == &s->positionCC)
			{
				updateOutput(s);
			}
		}
	}
	}
}

void ScreenOutputWatcher::startLoadFile()
{
	outputs.clear();
}

void ScreenOutputWatcher::endLoadFile()
{
	for (auto& s : ScreenManager::getInstance()->items) updateOutput(s);
}
