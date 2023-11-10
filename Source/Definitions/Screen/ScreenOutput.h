/*
  ==============================================================================

    ScreenOutput.h
    Created: 9 Nov 2023 8:51:23pm
    Author:  rosta

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class ScreenOutput :
    public juce::Component,
    public juce::OpenGLRenderer
{
    public:
    ScreenOutput();
    ~ScreenOutput();

    void paint(Graphics&) override;

    void goLive(int screenId);
    void stopLive();

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void createAndLoadShaders();
    BoolParameter* isOn = nullptr;

    static bool intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect); // should be in another objet

    std::unique_ptr<OpenGLShaderProgram> shader;
    juce::OpenGLContext openGLContext;
    Image myImage;
    std::shared_ptr<Image::BitmapData> bitmapData = nullptr;
    GLuint textureID;
    OpenGLTexture myTexture;
    Point<float> tl, tr, bl, br;
    Point<float> mousePos;
    Component* previousParent = nullptr;
    bool isLive = false;
};