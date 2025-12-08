/*
  ==============================================================================

	GLHelpers.h
	Created: 23 Nov 2023 12:27:26am
	Author:  bkupe

  ==============================================================================
*/

#pragma once




#define Init2DViewport(w, h) glViewport(0, 0, w, h); \
Init2DMatrix(w, h);

#define Init2DMatrix(w, h) glMatrixMode(GL_PROJECTION);\
glLoadIdentity(); \
glOrtho(0, w, 0, h, 0, 1); \
glMatrixMode(GL_MODELVIEW); \
glLoadIdentity(); \

#define Draw2DRect(x, y, w, h) glBegin(GL_QUADS); \
glVertex2f(x, y); \
glVertex2f(x + w, y); \
glVertex2f(x + w, y + h); \
glVertex2f(x, y + h); \
glEnd();

#define Draw2DTexRect(x, y, w, h) glBegin(GL_QUADS); \
glTexCoord2f(0, 0); glVertex2f(x, y); \
glTexCoord2f(1, 0); glVertex2f(x + w, y); \
glTexCoord2f(1, 1); glVertex2f(x + w, y + h); \
glTexCoord2f(0, 1); glVertex2f(x, y + h); \
glEnd();

#define Draw2DTexRectDepth(x, y, w, h, depth) glBegin(GL_QUADS); \
glTexCoord3f(0, 0, depth); glVertex2f(x, y); \
glTexCoord3f(1, 0, depth); glVertex2f(x + w, y); \
glTexCoord3f(1, 1, depth); glVertex2f(x + w, y + h); \
glTexCoord3f(0, 1, depth); glVertex2f(x, y + h); \
glEnd();

#define Draw2DTexRectFlipped(x, y, w, h) glBegin(GL_QUADS); \
glTexCoord2f(0, 1); glVertex2f(x, y); \
glTexCoord2f(1, 1); glVertex2f(x + w, y); \
glTexCoord2f(1, 0); glVertex2f(x + w, y + h); \
glTexCoord2f(0, 0); glVertex2f(x, y + h); \
glEnd();


#define Draw2DSubTexRect(x, y, w, h, s0, t0, s1, t1) glBegin(GL_QUADS); \
glTexCoord2f(s0, t0); glVertex2f(x, y); \
glTexCoord2f(s1, t0); glVertex2f(x + w, y); \
glTexCoord2f(s1, t1); glVertex2f(x + w, y + h); \
glTexCoord2f(s0, t1); glVertex2f(x, y + h); \
glEnd();