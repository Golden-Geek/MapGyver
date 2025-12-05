/*
  ==============================================================================

	OpenGLManager.h
	Created: 20 Nov 2023 3:10:36pm
	Author:  rosta

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"


class OpenGLSharedRenderer :
	public juce::OpenGLRenderer
{
public:
	OpenGLSharedRenderer(Component* component) ;
	~OpenGLSharedRenderer();

	OpenGLContext context;
	Component* component;
	Point<int> glInitSize;

	virtual void newOpenGLContextCreated() override;

	virtual void renderOpenGL() override = 0;
	virtual void openGLContextClosing() override = 0;
};

class GlContextHolder :
	private juce::ComponentListener,
	private juce::OpenGLRenderer
	//public juce::HighResolutionTimer
{
public:
	juce_DeclareSingleton(GlContextHolder, true);

	GlContextHolder();
	~GlContextHolder();

	double timeAtRender;

	juce::OpenGLContext context;
	juce::Component* parent;
	Component offScreenRenderComponent;
	CriticalSection renderLock;

	Array<OpenGLSharedRenderer*, juce::CriticalSection> sharedRenderers;

	void setup(juce::Component* topLevelComponent);
	void detach();


	void registerOpenGlRenderer(juce::OpenGLRenderer* child, int priority = 0);
	void unregisterOpenGlRenderer(juce::OpenGLRenderer* child);
	void registerSharedRenderer(OpenGLSharedRenderer* r);
	void unregisterSharedRenderer(OpenGLSharedRenderer* r);

	void callOnGLThread(std::function<void()> func, bool waitUntilDone = false)
	{
		context.executeOnGLThread([func](juce::OpenGLContext& ctx)
			{
				func();
			}, waitUntilDone);
	}
	class RenderTimerListener
	{
	public:
		virtual void renderCallback() {};
	};

	LightweightListenerList<RenderTimerListener> renderTimerListeners;
	void addRenderTimerListener(RenderTimerListener* listener) { renderTimerListeners.add(listener); }
	void removeRenderTimerListener(RenderTimerListener* listener) { renderTimerListeners.remove(listener); }

private:

	struct Client
	{
		enum class State
		{
			suspended,
			running
		};

		Client(juce::OpenGLRenderer* r, State nextStateToUse = State::suspended, int priority = 0)
			: r(r), c(dynamic_cast<Component*>(r)), currentState(State::suspended), nextState(nextStateToUse), glPriority(priority) {}



		juce::OpenGLRenderer* r = nullptr;
		juce::Component* c = nullptr;
		State currentState = State::suspended, nextState = State::suspended;
		int glPriority;
	};


	juce::CriticalSection stateChangeCriticalSection;
	juce::OwnedArray<Client, juce::CriticalSection> clients;

	void checkComponents(bool isClosing, bool isDrawing);

	void componentParentHierarchyChanged(juce::Component& component) override;
	void componentVisibilityChanged(juce::Component& component) override;
	void componentBeingDeleted(juce::Component& component) override;


	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	int findClientIndexForComponent(juce::Component* c) const;
	Client* findClientForComponent(juce::Component* c) const;
	int findClientIndexForRenderer(juce::OpenGLRenderer* r) const;
	Client* findClientForRenderer(juce::OpenGLRenderer* r) const;



};