/*
 * PseudoGL to GX Translations
 *
 * Enables you to use OpenGL functions instead of
 * GX functions to develop games for the wii
 *
 * Copyright (C) 2008-2009  Alejandro Valenzuela Roca
 *
 * lanjoe9@mexinetica.com
 *
 * This file is part of MotorJ, a free framework for videogame development.
 * <http://wiki.lidsol.net/wiki/index.php?title=MotorJ >
 * <http://motorj.mexinetica.com >
 *
 * MotorJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MotorJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MotorJ.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GL2GXTRANS_H
#define GL2GXTRANS_H

#include <gccore.h>
#include <ogc/gu.h>
//#include "app.h"
#define vector3 guVector
#include <math.h>


enum GLenum{GL_MODELVIEW, GL_PROJECTION,
            GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN,
            GL_POINTS, GL_QUADS,
            GL_LINES, GL_LINE_STRIP};//, GL_TEXTURE, GL_COLOR };

#define GL_TRUE GX_TRUE
#define GL_FALSE GX_FALSE
#define GL_ENABLE GX_ENABLE
#define GL_PERSPECTIVE GX_PERSPECTIVE
#define GL_ORTHO GX_ORTHOGRAPHIC


#define GLfloat	f32
#define GLint	int
#define GLuint	unsigned
#define u32	unsigned

#define glNormal3f(x,y,z) GX_Normal3f32(x,y,z)

#define glEnd() GX_End()

void mjWiiGLInit(Mtx44 * perspectiveMtxStack, Mtx44 * modelviewMtxStack, 
		 short maxPerspStack, short maxMvStack);

void mjWiiGLSetVs(unsigned numVertices);

void glMatrixMode(GLenum mode);


void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

void glTranslatef(GLfloat Tx, GLfloat Ty, GLfloat Tz);

void glScalef(GLfloat x, GLfloat y, GLfloat z);

void glBegin(GLenum mode);

void guTranslatef(guVector & t);

void glLoadIdentity();

void glPushMatrix();

void glPopMatrix();

void gluLookAt(GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
	       GLfloat centerX, GLfloat centerY, GLfloat centerZ,
	       GLfloat upX, GLfloat upY, GLfloat upZ);

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat alpha);

void glVertex3f(float x, float y, float z);

void glColor3f(float x, float y, float z);

#endif

