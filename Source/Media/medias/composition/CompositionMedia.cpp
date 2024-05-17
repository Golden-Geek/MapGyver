/*
  ==============================================================================

	CompositionMedia.cpp
	Created: 26 Sep 2020 1:51:42pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

CompositionMedia::CompositionMedia(var params) :
	Media(getTypeString(), params, true)
{
	backgroundColor = addColorParameter("Background Color", "Background Color", Colour::fromFloatRGBA(0, 0, 0, 0));

	addChildControllableContainer(&layers);
	alwaysRedraw = true;
}

CompositionMedia::~CompositionMedia()
{
}

void CompositionMedia::clearItem()
{
	BaseItem::clearItem();
}


void CompositionMedia::renderGLInternal()
{
	//frameBuffer.makeCurrentAndClear();
	Colour c = backgroundColor->getColor();
	glClearColor(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, frameBuffer.getWidth(), frameBuffer.getHeight(), 0, 0, 1);
	glEnable(GL_BLEND);

	for (int i = layers.items.size() - 1; i >= 0; i--)
	{
		CompositionLayer* l = layers.items[i];
		if(!l->enabled->boolValue()) continue;

		if (Media* m = dynamic_cast<Media*>(l->media))
		{
			if (!m->enabled->boolValue()) continue;

			GLenum sFactor = GL_SRC_ALPHA;
			GLenum dFactor = GL_ONE_MINUS_SRC_ALPHA;

			switch (l->blendFunctionSourceFactor->getValueDataAsEnum<CompositionLayer::blendOption>())
			{
			case CompositionLayer::ZERO: sFactor = GL_ZERO; break;
			case CompositionLayer::ONE: sFactor = GL_ONE; break;
			case CompositionLayer::SRC_ALPHA: sFactor = GL_SRC_ALPHA; break;
			case CompositionLayer::ONE_MINUS_SRC_ALPHA: sFactor = GL_ONE_MINUS_SRC_ALPHA; break;
			case CompositionLayer::DST_ALPHA: sFactor = GL_DST_ALPHA; break;
			case CompositionLayer::ONE_MINUS_DST_ALPHA: sFactor = GL_ONE_MINUS_DST_ALPHA; break;
			case CompositionLayer::SRC_COLOR: sFactor = GL_SRC_COLOR; break;
			case CompositionLayer::ONE_MINUS_SRC_COLOR: sFactor = GL_ONE_MINUS_SRC_COLOR; break;
			case CompositionLayer::DST_COLOR: sFactor = GL_DST_COLOR; break;
			case CompositionLayer::ONE_MINUS_DST_COLOR: sFactor = GL_ONE_MINUS_DST_COLOR; break;
			}

			switch (l->blendFunctionDestinationFactor->getValueDataAsEnum<CompositionLayer::blendOption>())
			{
			case CompositionLayer::ZERO: dFactor = GL_ZERO; break;
			case CompositionLayer::ONE: dFactor = GL_ONE; break;
			case CompositionLayer::SRC_ALPHA: dFactor = GL_SRC_ALPHA; break;
			case CompositionLayer::ONE_MINUS_SRC_ALPHA: dFactor = GL_ONE_MINUS_SRC_ALPHA; break;
			case CompositionLayer::DST_ALPHA: dFactor = GL_DST_ALPHA; break;
			case CompositionLayer::ONE_MINUS_DST_ALPHA: dFactor = GL_ONE_MINUS_DST_ALPHA; break;
			case CompositionLayer::SRC_COLOR: dFactor = GL_SRC_COLOR; break;
			case CompositionLayer::ONE_MINUS_SRC_COLOR: dFactor = GL_ONE_MINUS_SRC_COLOR; break;
			case CompositionLayer::DST_COLOR: dFactor = GL_DST_COLOR; break;
			case CompositionLayer::ONE_MINUS_DST_COLOR: dFactor = GL_ONE_MINUS_DST_COLOR; break;
			}

			glBlendFunc(sFactor, dFactor);

			int x = l->position->x;
			int y = l->position->y;
			int width = l->size->x;
			int height = l->size->y;

			// Rotation en radians (ex : M_PI_4 pour une rotation de 45 degrés)
			float rotationAngle = l->rotation->floatValue();

			// Niveau de transparence (0.0 pour complètement transparent, 1.0 pour complètement opaque)
			float alpha = l->alpha->floatValue();

			// Active la texture
			glBindTexture(GL_TEXTURE_2D, m->getTextureID());
			// Applique la rotation et la transparence
			glPushMatrix();
			glTranslatef(x + width / 2.0f, y + height / 2.0f, 0.0f);
			glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);
			glTranslatef(-width / 2.0f, -height / 2.0f, 0.0f);

			float yA = y;
			float yB = y + height;

			// Dessine le rectangle avec la texture
			glColor4f(1.0f, 1.0f, 1.0f, alpha);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x, yB);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, yB);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, yA);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x, yA);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			juce::gl::glPopMatrix();
		}

		// Restaure la matrice de modèle-vue
	}
	//frameBuffer.releaseAsRenderingTarget();

}


