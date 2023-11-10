/*
  ==============================================================================

    ScreenOutput.cpp
    Created: 9 Nov 2023 8:51:23pm
    Author:  rosta

  ==============================================================================
*/

#include "ScreenOutput.h"

using namespace juce::gl;

ScreenOutput::ScreenOutput()
{
    setOpaque(true);
    //myImage = ImageCache::getFromFile(File("C:\\Users\\rosta\\Pictures\\olga.jpeg"));
    myImage = ImageCache::getFromFile(File("C:\\Users\\rosta\\Pictures\\mire.png"));
    bitmapData = std::make_shared<Image::BitmapData>(myImage, Image::BitmapData::readOnly);

    tl = Point<float>(-1, 1);
    tr = Point<float>(1, 1);
    bl = Point<float>(-1, -1);
    br = Point<float>(1, -1);

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setComponentPaintingEnabled(true);

    OpenGLPixelFormat pixelFormat;
    pixelFormat.multisamplingLevel = 4; // Change this value to your needs.
    openGLContext.setPixelFormat(pixelFormat);
    stopLive();
}

ScreenOutput::~ScreenOutput()
{
    stopLive();
    openGLContext.detach();
}

void ScreenOutput::paint(Graphics& g)
{
}

void ScreenOutput::goLive(int screenId)
{
    Displays ds = Desktop::getInstance().getDisplays();

    if (isLive) { return; }
    if (screenId >= ds.displays.size()) { 
    stopLive();
        return; 
    }

    if (isOn != nullptr) {
        isOn->setValue(true);
    }

    isLive = true;
    Displays::Display d = ds.displays[screenId];
    openGLContext.setContinuousRepainting(true);
    //previousParent = getParentComponent();
    addToDesktop(0);
    setVisible(true);
    setBounds(d.totalArea);
    setAlwaysOnTop(true);
    repaint();
}

void ScreenOutput::stopLive()
{
    if (isOn != nullptr) {
        isOn->setValue(false);
    }
    if (!isLive) { return; }
    setSize(1, 1);
    openGLContext.triggerRepaint();
    openGLContext.setContinuousRepainting(false);
    isLive = false;
    setAlwaysOnTop(false);
    //setVisible(false);
    //removeFromDesktop();
    //setAlwaysOnTop(false);
    //previousParent->addAndMakeVisible(this);
    //previousParent->resized();
}

void ScreenOutput::newOpenGLContextCreated()
{
    // Set up your OpenGL state here

    if (!myImage.isNull())
    {
        myTexture.loadImage(myImage);
    }

    createAndLoadShaders();
}

void ScreenOutput::renderOpenGL()
{
    // Définir la vue OpenGL en fonction de la taille du composant
    if (!isLive) {return;}

    glViewport(0, 0, getWidth(), getHeight());

    if (shader != nullptr)
    {
        shader->use();

        myTexture.bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Point<float> center(0, 0);
        intersection(tl, br, bl, tr, &center);

        float dtl = center.getDistanceFrom(tl);
        float dtr = center.getDistanceFrom(tr);
        float dbr = center.getDistanceFrom(br);
        float dbl = center.getDistanceFrom(bl);

        float ztl = ((dtl + dbr) / dbr);
        float ztr = ((dtr + dbl) / dbl);
        float zbr = ((dbr + dtl) / dtl);
        float zbl = ((dbl + dtr) / dtr);

        // Define vertices for a triangle
        GLfloat vertices[] = {
            //  Position        Texcoords
                tl.x, tl.y, 0.0f, ztl, ztl, // Top-left
                tr.x, tr.y, ztr, ztr, ztr, // Top-right
                br.x, br.y, zbr, 0.0f, zbr, // Bottom-right
                bl.x, bl.y, 0.0f, 0.0f, zbl // Bottom-left
        };
        // Create a vertex buffer object
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLint posAttrib = glGetAttribLocation(shader->getProgramID(), "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

        GLint texAttrib = glGetAttribLocation(shader->getProgramID(), "texcoord");
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0,
        };

        GLuint ebo;
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
        myTexture.unbind();
    }
}

void ScreenOutput::openGLContextClosing()
{
    myTexture.unbind();
    myTexture.release();
    shader = nullptr;
}

void ScreenOutput::createAndLoadShaders()
{
    //String vertexShader, fragmentShader;

    const char* vertexShaderCode =
        "in vec2 position;\n"
        "in vec3 texcoord;\n"
        "out vec3 Texcoord;\n"
        "void main()\n"
        "{\n"
        "   Texcoord = texcoord;\n"
        "    gl_Position = vec4(position,0,1);\n"
        "}\n";

    const char* fragmentShaderCode =
        "in vec3 Texcoord;\n"
        "out vec4 outColor;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "    outColor = textureProj(tex, Texcoord);"//texture(myTexture, fragTextureCoord);\n"
        "    outColor[3] = 1.0f;"
        //"    outColor = vec4(0.5f,0.5f,0.5f,0.5f);   "
        "}\n";

    shader.reset(new OpenGLShaderProgram(openGLContext));
    shader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(vertexShaderCode));
    shader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fragmentShaderCode));
    shader->link();

}

bool ScreenOutput::intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect)
{
    // Store the values for fast access and easy
    // equations-to-code conversion
    float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) return false;

    // Get the x and y
    float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
    float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
    float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

    // Check if the x and y coordinates are within both lines
    if (x < jmin(x1, x2) || x > jmax(x1, x2) ||
        x < jmin(x3, x4) || x > jmax(x3, x4)) return false;
    if (y < jmin(y1, y2) || y > jmax(y1, y2) ||
        y < jmin(y3, y4) || y > jmax(y3, y4)) return false;

    // Return the point of intersection
    intersect->setXY(x, y);
    return true;
}
