/*
  ==============================================================================

    OpenGLManager.cpp
    Created: 20 Nov 2023 3:10:36pm
    Author:  rosta

  ==============================================================================
*/

#include "Common/CommonIncludes.h"

juce_ImplementSingleton(GlContextHolder)

GlContextHolder::GlContextHolder()
{
}

GlContextHolder::~GlContextHolder()
{
	detach();
}

void GlContextHolder::setup(juce::Component* topLevelComponent)
{
	parent = topLevelComponent;

	context.setRenderer(this);
	context.setContinuousRepainting(true);
	context.setComponentPaintingEnabled(true);
	context.attachTo(*parent);
}

//==============================================================================
// The context holder MUST explicitely call detach in their destructor

void GlContextHolder::detach()
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	const int n = clients.size();
	for (int i = 0; i < n; ++i)
		if (juce::Component* comp = clients[i]->c)
			comp->removeComponentListener(this);

	context.detach();
	context.setRenderer(nullptr);
}

//==============================================================================
// Clients MUST call unregisterOpenGlRenderer manually in their destructors!!

void GlContextHolder::registerOpenGlRenderer(juce::Component* child)
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	if (dynamic_cast<juce::OpenGLRenderer*> (child) != nullptr)
	{
		if (findClientIndexForComponent(child) < 0)
		{
			clients.add(new Client(child, (parent->isParentOf(child) ? Client::State::running : Client::State::suspended)));
			child->addComponentListener(this);
		}
	}
	else
		jassertfalse;
}

void GlContextHolder::unregisterOpenGlRenderer(juce::Component* child)
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	const int index = findClientIndexForComponent(child);

	if (index >= 0)
	{
		Client* client = clients[index];
		{
			juce::ScopedLock stateChangeLock(stateChangeCriticalSection);
			client->nextState = Client::State::suspended;
		}

		child->removeComponentListener(this);
		context.executeOnGLThread([this](juce::OpenGLContext&)
			{
				checkComponents(false, false);
			}, true);
		client->c = nullptr;

		clients.remove(index);
	}
}

void GlContextHolder::setBackgroundColour(const juce::Colour c)
{
	backgroundColour = c;
}

//==============================================================================

void GlContextHolder::checkComponents(bool isClosing, bool isDrawing)
{
	juce::Array<juce::Component*> initClients, runningClients;

	{
		juce::ScopedLock arrayLock(clients.getLock());
		juce::ScopedLock stateLock(stateChangeCriticalSection);

		const int n = clients.size();

		for (int i = 0; i < n; ++i)
		{
			Client* client = clients[i];
			if (client->c != nullptr)
			{
				Client::State nextState = (isClosing ? Client::State::suspended : client->nextState);

				if (client->currentState == Client::State::running && nextState == Client::State::running)   runningClients.add(client->c);
				else if (client->currentState == Client::State::suspended && nextState == Client::State::running)   initClients.add(client->c);
				else if (client->currentState == Client::State::running && nextState == Client::State::suspended)
				{
					dynamic_cast<juce::OpenGLRenderer*> (client->c)->openGLContextClosing();
				}

				client->currentState = nextState;
			}
		}
	}

	for (int i = 0; i < initClients.size(); ++i)
		dynamic_cast<juce::OpenGLRenderer*> (initClients.getReference(i))->newOpenGLContextCreated();

	if (runningClients.size() > 0 && isDrawing)
	{
		const float displayScale = static_cast<float> (context.getRenderingScale());
		const juce::Rectangle<int> parentBounds = (parent->getLocalBounds().toFloat() * displayScale).getSmallestIntegerContainer();

		for (int i = 0; i < runningClients.size(); ++i)
		{
			juce::Component* comp = runningClients.getReference(i);

			juce::Rectangle<int> r = (parent->getLocalArea(comp, comp->getLocalBounds()).toFloat() * displayScale).getSmallestIntegerContainer();
			glViewport((GLint)r.getX(),
				(GLint)parentBounds.getHeight() - (GLint)r.getBottom(),
				(GLsizei)r.getWidth(), (GLsizei)r.getHeight());
			juce::OpenGLHelpers::clear(backgroundColour);

			dynamic_cast<juce::OpenGLRenderer*> (comp)->renderOpenGL();
		}
	}
}

//==============================================================================

void GlContextHolder::componentParentHierarchyChanged(juce::Component& component)
{
	if (Client* client = findClientForComponent(&component))
	{
		juce::ScopedLock stateChangeLock(stateChangeCriticalSection);

		client->nextState = (parent->isParentOf(&component) && component.isVisible() ? Client::State::running : Client::State::suspended);
	}
}

void GlContextHolder::componentVisibilityChanged(juce::Component& component)
{
	if (Client* client = findClientForComponent(&component))
	{
		juce::ScopedLock stateChangeLock(stateChangeCriticalSection);

		client->nextState = (parent->isParentOf(&component) && component.isVisible() ? Client::State::running : Client::State::suspended);
	}
}

void GlContextHolder::componentBeingDeleted(juce::Component& component)
{
	const int index = findClientIndexForComponent(&component);

	if (index >= 0)
	{
		Client* client = clients[index];

		// You didn't call unregister before deleting this component
		jassert(client->nextState == Client::State::suspended);
		client->nextState = Client::State::suspended;

		component.removeComponentListener(this);
		context.executeOnGLThread([this](juce::OpenGLContext&)
			{
				checkComponents(false, false);
			}, true);

		client->c = nullptr;

		clients.remove(index);
	}
}

//==============================================================================

void GlContextHolder::newOpenGLContextCreated()
{
	gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
	glDisable(GL_DEBUG_OUTPUT);
	checkComponents(false, false);
}

void GlContextHolder::renderOpenGL()
{
	juce::OpenGLHelpers::clear(backgroundColour);
	checkComponents(false, true);
}

void GlContextHolder::openGLContextClosing()
{
	checkComponents(true, false);
}
