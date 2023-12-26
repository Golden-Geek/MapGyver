/*
  ==============================================================================

    OpenGLManager.cpp
    Created: 20 Nov 2023 3:10:36pm
    Author:  rosta

  ==============================================================================
*/

#include "Common/CommonIncludes.h"
#include "Engine/RMPEngine.h"

juce_ImplementSingleton(GlContextHolder)

GlContextHolder::GlContextHolder() :
	timeAtRender(0)
{
}

GlContextHolder::~GlContextHolder()
{
	detach();
}

void GlContextHolder::setup(juce::Component* topLevelComponent)
{
	parent = topLevelComponent;
	if(OpenGLRenderer* r = dynamic_cast<OpenGLRenderer*>(parent)) registerOpenGlRenderer(r);

	context.setSwapInterval(0);
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

void GlContextHolder::registerOpenGlRenderer(juce::OpenGLRenderer* child)
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	if (dynamic_cast<juce::OpenGLRenderer*> (child) != nullptr)
	{
		if (findClientIndexForRenderer(child) < 0)
		{
			juce::Component* c = dynamic_cast<juce::Component*> (child);
			Client::State state = Client::State::running;
			if (c != nullptr) state = (parent == c || parent->isParentOf(c)) ? Client::State::running : Client::State::suspended;
			clients.add(new Client(child, state));
			if(c != nullptr) c->addComponentListener(this);
		}
	}
	else
		jassertfalse;
}

void GlContextHolder::unregisterOpenGlRenderer(juce::OpenGLRenderer* child)
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	const int index = findClientIndexForRenderer(child);

	if (index >= 0)
	{
		Client* client = clients[index];
		{
			juce::ScopedLock stateChangeLock(stateChangeCriticalSection);
			client->nextState = Client::State::suspended;
		}

		if(client->c!= nullptr) client->c->removeComponentListener(this);
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
	juce::Array<Client*> initClients, runningClients;

	{
		juce::ScopedLock arrayLock(clients.getLock());
		juce::ScopedLock stateLock(stateChangeCriticalSection);

		const int n = clients.size();

		for (int i = 0; i < n; ++i)
		{
			Client* client = clients[i];
			if (client->r != nullptr)
			{
				Client::State nextState = (isClosing ? Client::State::suspended : client->nextState);

				if (client->currentState == Client::State::running && nextState == Client::State::running)   runningClients.add(client);
				else if (client->currentState == Client::State::suspended && nextState == Client::State::running)   initClients.add(client);
				else if (client->currentState == Client::State::running && nextState == Client::State::suspended)
				{
					client->r->openGLContextClosing();
				}

				client->currentState = nextState;
			}
		}
	}

	for (int i = 0; i < initClients.size(); ++i)
		initClients.getReference(i)->r->newOpenGLContextCreated();

	if (runningClients.size() > 0 && isDrawing)
	{
		const float displayScale = static_cast<float> (context.getRenderingScale());
		const juce::Rectangle<int> parentBounds = (parent->getLocalBounds().toFloat() * displayScale).getSmallestIntegerContainer();

		for (int i = 0; i < runningClients.size(); ++i)
		{
			Client* rc = runningClients.getReference(i);
			juce::Component* comp = rc->c;

			if (comp != nullptr)
			{
				juce::Rectangle<int> r = (parent->getLocalArea(comp, comp->getLocalBounds()).toFloat() * displayScale).getSmallestIntegerContainer();
				glViewport((GLint)r.getX(),
					(GLint)parentBounds.getHeight() - (GLint)r.getBottom(),
					(GLsizei)r.getWidth(), (GLsizei)r.getHeight());
			}
			//juce::OpenGLHelpers::clear(backgroundColour);

			rc->r->renderOpenGL();
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
#if JUCE_WINDOWS
	gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
	glDisable(GL_DEBUG_OUTPUT);
#endif
	checkComponents(false, false);
}

void GlContextHolder::renderOpenGL()
{
	timeAtRender = Time::getMillisecondCounterHiRes();

	juce::OpenGLHelpers::clear(backgroundColour);
	checkComponents(false, true);
}

void GlContextHolder::openGLContextClosing()
{
	checkComponents(true, false);
}
