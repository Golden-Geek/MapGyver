/*
  ==============================================================================

    OpenGLManager.cpp
    Created: 20 Nov 2023 3:10:36pm
    Author:  rosta

  ==============================================================================
*/

#include "OpenGLManager.h"

juce_ImplementSingleton(OpenGLManager);

OpenGLManager::OpenGLManager() {

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setComponentPaintingEnabled(true);

    openGLContext.setContinuousRepainting(true);
    addToDesktop(0);
    setBounds(-1,0,1,1);
    setVisible(true);
}

OpenGLManager::~OpenGLManager() {

}

void OpenGLManager::newOpenGLContextCreated()
{
    gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_API, gl::GL_DEBUG_TYPE_OTHER, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, gl::GL_FALSE);
    glDisable(GL_DEBUG_OUTPUT);

}

void OpenGLManager::renderOpenGL()
{
    elements.call(&OpenGLManager::OpenGLElement::render);
}

void OpenGLManager::openGLContextClosing()
{
}

void OpenGLManager::addOpenGLElement(OpenGLElement* newListener)
{
    elements.add(newListener);

}

void OpenGLManager::removeOpenGLElement(OpenGLElement* listener)
{
    elements.remove(listener);
}

