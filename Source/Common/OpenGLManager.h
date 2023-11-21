/*
  ==============================================================================

	OpenGLManager.h
	Created: 20 Nov 2023 3:10:36pm
	Author:  rosta

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class GlContextHolder :
	private juce::ComponentListener,
	private juce::OpenGLRenderer
{
public:
	juce_DeclareSingleton(GlContextHolder, true);

	GlContextHolder();
	~GlContextHolder();

	void setup(juce::Component* topLevelComponent);

	//==============================================================================
	// The context holder MUST explicitely call detach in their destructor
	void detach();

	//==============================================================================
	// Clients MUST call unregisterOpenGlRenderer manually in their destructors!!
	void registerOpenGlRenderer(juce::Component* child);

	void unregisterOpenGlRenderer(juce::Component* child);

	void setBackgroundColour(const juce::Colour c);

	juce::OpenGLContext context;

private:
	//==============================================================================
	void checkComponents(bool isClosing, bool isDrawing);

	//==============================================================================
	void componentParentHierarchyChanged(juce::Component& component) override;

	void componentVisibilityChanged(juce::Component& component) override;

	void componentBeingDeleted(juce::Component& component) override;

	//==============================================================================
	void newOpenGLContextCreated() override;

	void renderOpenGL() override;

	void openGLContextClosing() override;

	//==============================================================================
	juce::Component* parent;

	struct Client
	{
		enum class State
		{
			suspended,
			running
		};

		Client(juce::Component* comp, State nextStateToUse = State::suspended)
			: c(comp), currentState(State::suspended), nextState(nextStateToUse) {}


		juce::Component* c = nullptr;
		State currentState = State::suspended, nextState = State::suspended;
	};

	juce::CriticalSection stateChangeCriticalSection;
	juce::OwnedArray<Client, juce::CriticalSection> clients;

	//==============================================================================
	int findClientIndexForComponent(juce::Component* comp) const
	{
		const int n = clients.size();
		for (int i = 0; i < n; ++i)
			if (comp == clients[i]->c)
				return i;

		return -1;
	}

	Client* findClientForComponent(juce::Component* comp) const
	{
		const int index = findClientIndexForComponent(comp);
		if (index >= 0 && index < clients.size())
			return clients[index];

		return nullptr;
	}

	//==============================================================================
	juce::Colour backgroundColour{ juce::Colours::black };
};