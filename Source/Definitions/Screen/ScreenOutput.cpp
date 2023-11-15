/*
  ==============================================================================

    ScreenOutput.cpp
    Created: 9 Nov 2023 8:51:23pm
    Author:  rosta

  ==============================================================================
*/

#include "ScreenOutput.h"
#include "Screen.h"
#include "Definitions/Media/Media.h"

using namespace juce::gl;

ScreenOutput::ScreenOutput(Screen* parent)
{
    setOpaque(true);
    parentScreen = parent;

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setComponentPaintingEnabled(false);

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
        LOGERROR("selected display is not available");
        stopLive();
        return; 
    }

    if (parentScreen->enabled != nullptr) {
        parentScreen->enabled->setValue(true);
    }

    isLive = true;
    Displays::Display d = ds.displays[screenId];
    openGLContext.setContinuousRepainting(true);
    //previousParent = getParentComponent();
    Rectangle<int> a = d.totalArea;
    a.setWidth(a.getWidth() / 2);
    a.setHeight(a.getHeight() / 2);
    addToDesktop(0);
    setVisible(true);
    setBounds(a);
    setAlwaysOnTop(true);
    repaint();
}

void ScreenOutput::stopLive()
{
    if (parentScreen->enabled != nullptr) {
        //parentScreen->enabled->setValue(false);
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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (parentScreen != nullptr) {
            Point<float> tl, tr, bl, br;
            for (int i = 0; i < parentScreen->surfaces.items.size(); i++) {
                Surface* s = parentScreen->surfaces.items[i];
                if (s->enabled->boolValue()) {
                    Media* m = dynamic_cast<Media*>(s->media->targetContainer.get());
                    std::shared_ptr<OpenGLTexture> tex = nullptr;

                    //myTexture.bind();
                    if (m != nullptr) {
                        if (!textures.contains(m)) {
                            tex = std::make_shared<OpenGLTexture>();
                            tex->loadImage(m->myImage);
                            texturesVersions.set(m, m->imageVersion);
                            textures.set(m, tex);
                        }
                        tex = textures.getReference(m);
                        unsigned int vers = texturesVersions.getReference(m);
                        if (m->imageVersion != vers) {
                            tex->loadImage(m->myImage);
                            texturesVersions.set(m, m->imageVersion);
                        }
                        tex->bind();
                    }

                    tl.setXY(s->topLeft->x, s->topLeft->y);
                    tr.setXY(s->topRight->x, s->topRight->y);
                    bl.setXY(s->bottomLeft->x, s->bottomLeft->y);
                    br.setXY(s->bottomRight->x, s->bottomRight->y);

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

                    Vector3D<float> tlTex(s->cropLeft->floatValue(), 1-s->cropTop->floatValue(),1.0f);
                    Vector3D<float> trTex(1- s->cropRight->floatValue(), 1-s->cropTop->floatValue(),1.0f);
                    Vector3D<float> blTex(s->cropLeft->floatValue(), s->cropBottom->floatValue(),1.0f);
                    Vector3D<float> brTex(1 - s->cropRight->floatValue(), s->cropBottom->floatValue(),1.0f);
                    
                    tlTex *= ztl;
                    trTex *= ztr;
                    blTex *= zbl;
                    brTex *= zbr;

                    // Define vertices for a triangle
                    GLfloat vertices[] = {
                        tl.x, tl.y, -1.0f, 1.0f, tlTex.x, tlTex.y, tlTex.z, // Top-left
                        tr.x, tr.y, 1.0f, 1.0f, trTex.x, trTex.y, trTex.z, // Top-right
                        br.x, br.y, 1.0f, -1.0f, brTex.x, brTex.y, brTex.z, // Bottom-right
                        bl.x, bl.y, -1.0f, -1.0f, blTex.x, blTex.y, blTex.z // Bottom-left
                    };
                    // Create a vertex buffer object
                    GLuint vbo;
                    glGenBuffers(1, &vbo);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                    GLint posAttrib = glGetAttribLocation(shader->getProgramID(), "position");
                    glEnableVertexAttribArray(posAttrib);
                    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

                    GLint surfacePosAttrib = glGetAttribLocation(shader->getProgramID(), "surfacePosition");
                    glEnableVertexAttribArray(surfacePosAttrib);
                    glVertexAttribPointer(surfacePosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(float)));

                    GLint texAttrib = glGetAttribLocation(shader->getProgramID(), "texcoord");
                    glEnableVertexAttribArray(texAttrib);
                    glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));

                    GLuint elements[] = {
                        0, 1, 2,
                        2, 3, 0,
                    };

                    GLuint ebo;
                    glGenBuffers(1, &ebo);

                    GLuint borderSoftLocation = glGetUniformLocation(shader->getProgramID(), "borderSoft");
                    glUniform4f(borderSoftLocation, s->softEdgeTop->floatValue(), s->softEdgeRight->floatValue(), s->softEdgeBottom->floatValue(), s->softEdgeLeft->floatValue());

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                
                    if (tex != nullptr) {
                        tex->unbind();
                    }

                }

            }
        }
        glDisable(GL_BLEND);
    }
}

void ScreenOutput::openGLContextClosing()
{
    glEnable(GL_BLEND);
    textures.clear();
    glDisable(GL_BLEND);
    shader = nullptr;
}

void ScreenOutput::createAndLoadShaders()
{
    //String vertexShader, fragmentShader;

    const char* vertexShaderCode =
        "in vec2 position;\n"
        "in vec2 surfacePosition;\n"
        "in vec3 texcoord;\n"
        "out vec3 Texcoord;\n"
        "out vec2 SurfacePosition;\n"
        "void main()\n"
        "{\n"
        "    Texcoord = texcoord;\n"
        "    SurfacePosition[0] = (surfacePosition[0]+1.0f)/2.0f;\n"
        "    SurfacePosition[1] = (surfacePosition[1]+1.0f)/2.0f;\n"
        "    gl_Position = vec4(position,0,1);\n"
        "}\n";

    const char* fragmentShaderCode =
        "in vec3 Texcoord;\n"
        "in vec2 SurfacePosition;\n"
        "out vec4 outColor;\n"
        "uniform sampler2D tex;\n"
        "uniform vec4 borderSoft;\n"
        "float map(float value, float min1, float max1, float min2, float max2) {\n"
        "return min2 + ((max2-min2)*(value-min1)/(max1-min1)); };"
        "void main()\n"
        "{\n"
        "    outColor = textureProj(tex, Texcoord);\n"
        "   float alpha = 1.0f;\n"
        "   if (SurfacePosition[1] > 1-borderSoft[0])    {alpha *= map(SurfacePosition[1],1.0f,1-borderSoft[0],0.0f,1.0f);}\n" // top
        "   if (SurfacePosition[0] > 1.0f-borderSoft[1]) {alpha *= map(SurfacePosition[0],1.0f,1-borderSoft[1],0.0f,1.0f);}\n" // right
        "   if (SurfacePosition[1] < borderSoft[2])      {alpha *= map(SurfacePosition[1],0.0f,borderSoft[2],0.0f,1.0f);}\n" // bottom
        "   if (SurfacePosition[0] < borderSoft[3])      {alpha *= map(SurfacePosition[0],0.0f,borderSoft[3],0.0f,1.0f);}\n" // left
        "    outColor[3] = alpha;\n"
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
