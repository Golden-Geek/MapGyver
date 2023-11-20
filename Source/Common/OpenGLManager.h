/*
  ==============================================================================

    OpenGLManager.h
    Created: 20 Nov 2023 3:10:36pm
    Author:  rosta

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class OpenGLManager :
    public OpenGLRenderer,
	public Component
{
public:
	juce_DeclareSingleton(OpenGLManager, true);
	OpenGLManager();
	~OpenGLManager();

	OpenGLContext openGLContext;
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	class OpenGLElement
	{
	public:
		/** Destructor. */
		virtual ~OpenGLElement() {}
		virtual void render() {}
	};

	ListenerList<OpenGLElement> elements;
	void addOpenGLElement(OpenGLElement* newListener);
	void removeOpenGLElement(OpenGLElement* listener);

};