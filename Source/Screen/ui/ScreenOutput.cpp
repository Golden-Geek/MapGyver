/*
  ==============================================================================

	ScreenOutput.cpp
	Created: 9 Nov 2023 8:51:23pm
	Author:  rosta

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Common/CommonIncludes.h"

juce_ImplementSingleton(ScreenOutputWatcher)

using namespace juce::gl;

ScreenOutput::ScreenOutput(Screen* screen) :
	InspectableContentComponent(screen),
	isLive(false),
	screen(screen)
{
	setOpaque(true);

	openGLContext.setNativeSharedContext(GlContextHolder::getInstance()->context.getRawContext());
	openGLContext.setRenderer(this);
	openGLContext.attachTo(*this);
	openGLContext.setComponentPaintingEnabled(true);

	setWantsKeyboardFocus(true); // Permet à ce composant de recevoir le focus clavier
	addKeyListener(this);        // Ajoutez ce composant comme écouteur clavier

	update();

}

ScreenOutput::~ScreenOutput()
{
	removeFromDesktop();
	openGLContext.detach();
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

		if (!prevIsLive)
		{
			openGLContext.setContinuousRepainting(true);
			addToDesktop(0);
			setAlwaysOnTop(true);
		}

		Rectangle<int> a = d.totalArea;
		a.setWidth(a.getWidth());
		a.setHeight(a.getHeight() - 1); // -1 to stop my screen to flicker ><
		setBounds(a);
		repaint();
	}
	else
	{
		if (prevIsLive)
		{
			removeFromDesktop();
			openGLContext.setContinuousRepainting(false);
			setAlwaysOnTop(false);
		}

		openGLContext.triggerRepaint();
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
	openGLContext.makeActive();

	glViewport(0, 0, getWidth(), getHeight());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, getWidth(), getHeight(), 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, screen->renderer->frameBuffer.getTextureID());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(0, 1); glVertex2f(0, getHeight());
	glTexCoord2f(1, 1); glVertex2f(getWidth(), getHeight());
	glTexCoord2f(1, 0); glVertex2f(getWidth(), 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glFinish();
}

void ScreenOutput::openGLContextClosing()
{
}

void ScreenOutput::userTriedToCloseWindow()
{
	if (inspectable.wasObjectDeleted()) return;
	screen->enabled->setValue(false);
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
		if (shouldShow) outputs.add(new ScreenOutput(s));
	}
	else
	{
		if (!shouldShow) outputs.removeObject(o);
		else o->update();
	}
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
		updateOutput(e.getItem());
		break;

	case ScreenManager::ManagerEvent::ITEMS_REMOVED:
		for (auto& s : e.getItems()) updateOutput(s);
		break;
	}
}

//void ScreenOutputWatcher::itemAdded(Screen* item)
//{
//	updateOutput(item);
//}
//
//void ScreenOutputWatcher::itemsAdded(Array<Screen*> items)
//{
//	for (auto& s : items) updateOutput(s);
//}
//
//void ScreenOutputWatcher::itemRemoved(Screen* item)
//{
//	updateOutput(item, true);
//}
//
//void ScreenOutputWatcher::itemsRemoved(Array<Screen*> items)
//{
//	for (auto& s : items) updateOutput(s, true);
//}

void ScreenOutputWatcher::newMessage(const ContainerAsyncEvent& e)
{
	if (Engine::mainEngine->isLoadingFile) return;

	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
	{
		if (Screen* s = dynamic_cast<Screen*>(e.targetControllable->parentContainer.get()))
		{
			if (e.targetControllable == s->enabled || e.targetControllable == s->outputType || e.targetControllable == s->screenID)
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
