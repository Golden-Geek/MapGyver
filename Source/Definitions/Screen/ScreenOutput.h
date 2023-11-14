/*
  ==============================================================================

    ScreenOutput.h
    Created: 9 Nov 2023 8:51:23pm
    Author:  rosta

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
class Screen; 
class Media;

class ScreenOutput :
    public juce::Component,
    public juce::OpenGLRenderer
{
    public:
    ScreenOutput(Screen* parent);
    ~ScreenOutput();

    Screen* parentScreen;

    void paint(Graphics&) override;

    void goLive(int screenId);
    void stopLive();

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void createAndLoadShaders();

    static bool intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect); // should be in another objet

    std::unique_ptr<OpenGLShaderProgram> shader;
    juce::OpenGLContext openGLContext;
    Point<float> mousePos;
    Component* previousParent = nullptr;
    bool isLive = false;

    HashMap<Media*, std::shared_ptr<OpenGLTexture>> textures;
    HashMap<Media*, int> texturesVersions;
};