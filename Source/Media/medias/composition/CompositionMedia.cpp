/*
  ==============================================================================

	CompositionMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

CompositionMedia::CompositionMedia(var params) :
	Media(getTypeString(), params)
{
	resolution = addPoint2DParameter("Resolution", "Size of your composition");
	var d; d.append(1920); d.append(1080);
	resolution->setDefaultValue(d);

	addChildControllableContainer(&layers);

	updateImagesSize();

	//startThread(Thread::Priority::highest);
}

CompositionMedia::~CompositionMedia()
{
	//stopThread(1000);
}

void CompositionMedia::clearItem()
{
	BaseItem::clearItem();
}

void CompositionMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == resolution) {
		updateImagesSize();
	}
}

void CompositionMedia::renderOpenGL()
{
    frameBuffer.makeCurrentAndClear();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, frameBuffer.getWidth(), frameBuffer.getHeight(), 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < layers.items.size(); i++) {
        CompositionLayer* l = layers.items[i];
        Media* m = dynamic_cast<Media*>(l->media->targetContainer.get());
        if (m != nullptr) {
            int x = l->position->x;
            int y = l->position->y;
            int width = l->size->x;
            int height = l->size->y;

            // Rotation en radians (ex : M_PI_4 pour une rotation de 45 degrés)
            float rotationAngle = l->rotation->floatValue();

            // Niveau de transparence (0.0 pour complètement transparent, 1.0 pour complètement opaque)
            float alpha = l->alpha->floatValue();

            // Active la texture
            glBindTexture(GL_TEXTURE_2D, m->frameBuffer.getTextureID());
            // Applique la rotation et la transparence
            glPushMatrix();
            glTranslatef(x + width / 2.0f, y + height / 2.0f, 0.0f);
            glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);
            glTranslatef(-width / 2.0f, -height / 2.0f, 0.0f);

            // Dessine le rectangle avec la texture
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y+height);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y+height);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y);
            glEnd();
        }

        // Restaure la matrice de modèle-vue
        juce::gl::glPopMatrix();
    }
    frameBuffer.releaseAsRenderingTarget();
}

void CompositionMedia::updateImagesSize()
{
    GlContextHolder::getInstance()->context.executeOnGLThread([this](OpenGLContext &c){
        frameBuffer.initialise(GlContextHolder::getInstance()->context, resolution->x, resolution->y);
    }, true);
    
}

void CompositionMedia::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	imageNeedRepaint = true;
}

