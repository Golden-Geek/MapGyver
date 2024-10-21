/*
  ==============================================================================

	OpenGLManager.cpp
	Created: 20 Nov 2023 3:10:36pm
	Author:  rosta

  ==============================================================================
*/

#include "Common/CommonIncludes.h"
#include "Engine/MGEngine.h"

juce_ImplementSingleton(GlContextHolder)

using namespace juce::gl;

GlContextHolder::GlContextHolder() :
	timeAtRender(0)
{
	offScreenRenderComponent.setSize(1, 1); // (1, 1) is the minimum size for an OpenGL context (on Windows at least
}

GlContextHolder::~GlContextHolder()
{
	detach();
}

void GlContextHolder::setup(juce::Component* topLevelComponent)
{
	topLevelComponent->addAndMakeVisible(offScreenRenderComponent);
	parent = topLevelComponent;
	//context.setOpenGLVersionRequired(juce::OpenGLContext::OpenGLVersion::openGL4_1);

	//if (OpenGLRenderer* r = dynamic_cast<OpenGLRenderer*>(offScreenRenderComponent)) registerOpenGlRenderer(r);
	//registerOpenGlRenderer(&offScreenRenderComponent);
	context.setSwapInterval(0);
	context.setRenderer(this);
	context.setContinuousRepainting(true);
	context.setComponentPaintingEnabled(true);
	context.attachTo(offScreenRenderComponent);
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

void GlContextHolder::registerOpenGlRenderer(juce::OpenGLRenderer* child, int priority)
{
	jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

	if (dynamic_cast<juce::OpenGLRenderer*> (child) != nullptr)
	{
		if (findClientIndexForRenderer(child) < 0)
		{
			juce::Component* c = dynamic_cast<juce::Component*> (child);
			Client::State state = Client::State::running;
			if (c != nullptr) state = (parent == c || parent->isParentOf(c)) ? Client::State::running : Client::State::suspended;
			clients.add(new Client(child, state, priority));
			std::sort(clients.begin(), clients.end(), [](const Client* a, const Client* b) { return a->glPriority < b->glPriority; });
			if (c != nullptr) c->addComponentListener(this);
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

		if (client->c != nullptr) client->c->removeComponentListener(this);
		context.executeOnGLThread([this](juce::OpenGLContext&)
			{
				checkComponents(false, false);
			}, true);
		client->c = nullptr;

		clients.remove(index);
	}
}

void GlContextHolder::registerSharedRenderer(OpenGLSharedRenderer* r, int delayBeforeAttach)
{
	//GenericScopedLock lock(renderLock);

	r->context.detach();

	r->context.setSwapInterval(0);
	r->context.setRenderer(r);

	context.executeOnGLThread([this, r](OpenGLContext& callerContext) {
		r->context.setNativeSharedContext(context.getRawContext());
		}, true);


	std::function<void()> attachFunc = [this, r]() {
		if (r->useSizeTrick)
		{
			r->glInitSize = Point<int>(r->component->getWidth(), r->component->getHeight());
			r->component->setSize(1, 1);
		}

		r->context.attachTo(*r->component);
		sharedRenderers.add(r);
		};

	if (delayBeforeAttach == 0) attachFunc();
	else Timer::callAfterDelay(delayBeforeAttach, attachFunc);
}

void GlContextHolder::unregisterSharedRenderer(OpenGLSharedRenderer* r)
{
	//GenericScopedLock lock(renderLock);
	sharedRenderers.removeAllInstancesOf(r);
	r->context.detach();
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
	const char* version = (const char*)glGetString(GL_VERSION);
	const char* vendor = (const char*)glGetString(GL_VENDOR);
	const char* renderer = (const char*)glGetString(GL_RENDERER);

	String openGLInfo = "OpenGL Version: " + String(version) + "\n"
		"Vendor: " + String(vendor) + "\n"
		"Renderer: " + String(renderer);

	LOG("OpenGL init :\n" << openGLInfo);

#if JUCE_WINDOWS
	gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
	glDisable(GL_DEBUG_OUTPUT);
#endif
	checkComponents(false, false);
}

void GlContextHolder::renderOpenGL()
{
	double lastRenderTime = timeAtRender;

	double t = Time::getMillisecondCounterHiRes();
	const double frameTime = 1000.0 / RMPSettings::getInstance()->fpsLimit->intValue();
	if (t - lastRenderTime < frameTime) return;

	timeAtRender = t;

	//LOG("*** Render Main GL >>");

	juce::OpenGLHelpers::clear(Colours::black);
	checkComponents(false, true);

	for (auto& c : sharedRenderers) c->context.triggerRepaint();
}

void GlContextHolder::openGLContextClosing()
{
	checkComponents(true, false);
}

//==============================================================================
// CLIENT
//==============================================================================

int GlContextHolder::findClientIndexForComponent(juce::Component* c) const
{
	const int n = clients.size();
	for (int i = 0; i < n; ++i)
		if (c == clients[i]->c)
			return i;

	return -1;
}

GlContextHolder::Client* GlContextHolder::findClientForComponent(juce::Component* c) const
{
	const int index = findClientIndexForComponent(c);
	if (index >= 0 && index < clients.size())
		return clients[index];

	return nullptr;
}

int GlContextHolder::findClientIndexForRenderer(juce::OpenGLRenderer* r) const
{
	const int n = clients.size();
	for (int i = 0; i < n; ++i)
		if (r == clients[i]->r)
			return i;

	return -1;
}

GlContextHolder::Client* GlContextHolder::findClientForRenderer(juce::OpenGLRenderer* r) const
{
	const int index = findClientIndexForRenderer(r);
	if (index >= 0 && index < clients.size())
		return clients[index];

	return nullptr;
}


void OpenGLSharedRenderer::registerRenderer(int delay)
{
	GlContextHolder::getInstance()->registerSharedRenderer(this, delay);
}

void OpenGLSharedRenderer::unregisterRenderer()
{
	if (GlContextHolder::getInstanceWithoutCreating()) GlContextHolder::getInstance()->unregisterSharedRenderer(this);
}

void OpenGLSharedRenderer::newOpenGLContextCreated()
{
	juce::gl::glDebugMessageControl(juce::gl::GL_DEBUG_SOURCE_API, juce::gl::GL_DEBUG_TYPE_OTHER, juce::gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, juce::gl::GL_FALSE);
	juce::gl::glDisable(juce::gl::GL_DEBUG_OUTPUT);

	if (useSizeTrick)
	{
		Timer::callAfterDelay(50, [this]() {
			component->setSize(glInitSize.x, glInitSize.y);
			});
	}
}
