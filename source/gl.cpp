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

#include "gl.h"

static guVector mjWiiCurrentColor = {1.0f, 0.0f, 0.0f};

typedef struct {
  short currentMM; // 0: perspective; 1: modelView
  short index[2];
  short max[2];
  short * currentIndex;
  Mtx44 * currentMtx;
  Mtx44 * stacks[2];
  unsigned numVertices;
  u32 SCCZvalue;
} mjWiiGLStruct;

static mjWiiGLStruct mjWiiGL;

void mjWiiGLInit(Mtx44 * perspectiveMtxStack, Mtx44 * modelviewMtxStack, short maxPerspStack, short maxMvStack)
{
    mjWiiGL.index[0] = 0;
    mjWiiGL.index[1] = 0;
    mjWiiGL.stacks[0] = perspectiveMtxStack;
    mjWiiGL.stacks[1] = modelviewMtxStack;
    mjWiiGL.currentMtx = &mjWiiGL.stacks[0][0];
    mjWiiGL.currentMM = 0;
    mjWiiGL.max[0] = maxPerspStack;
    mjWiiGL.max[1] = maxMvStack;
    guMtxIdentity(perspectiveMtxStack[0]);
    guMtxIdentity(modelviewMtxStack[0]);
    mjWiiGL.SCCZvalue = 0x00ffffff;
}



void glMatrixMode(GLenum mode)
{
  switch(mode)
    {
    case GL_PROJECTION:
      mjWiiGL.currentMM = 0;
      break;
    case GL_MODELVIEW:
      // Commit changes in projection matrix to Wii memory
      if (mjWiiGL.currentMM == 0)
	GX_LoadProjectionMtx(*mjWiiGL.currentMtx, GX_PERSPECTIVE);
      mjWiiGL.currentMM = 1;
      break;
    default:
      //FIXME: generate error
      break;
    }
  mjWiiGL.currentMtx = &mjWiiGL.stacks[mjWiiGL.currentMM][mjWiiGL.index[mjWiiGL.currentMM]];
  
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  Mtx44 rotMtx;
  //Mtx44 tempMtx;
  
  guVector tmp;
  tmp.x = x;
  tmp.y = y;
  tmp.z = z;
  guMtxRotAxisDeg(rotMtx, &tmp, angle);
  guMtxConcat(*mjWiiGL.currentMtx, rotMtx, *mjWiiGL.currentMtx);

  
}

void glTranslatef(GLfloat Tx, GLfloat Ty, GLfloat Tz)
{
	guMtxApplyTrans(*mjWiiGL.currentMtx, *mjWiiGL.currentMtx, Tx, Ty, Tz);
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	guMtxApplyScale(*mjWiiGL.currentMtx,*mjWiiGL.currentMtx, x, y, z);
}

void mjWiiGLSetVs(unsigned numVertices)
{
	mjWiiGL.numVertices = numVertices;
}

void glBegin(GLenum mode)
{
    //Fixme: bad implementation
    //Fixme: more formats/sizes ?
    //Fixme: return error on bad mode
  
  // Load the current matrix into memory
  GX_LoadPosMtxImm(*mjWiiGL.currentMtx, GX_PNMTX0);
  // now perform the BEGIN
  // Note the numVertices variable MUST be set previously, using mjWiiGLSetVs
  switch(mode)
    {
    case GL_TRIANGLES:
      GX_Begin(GX_TRIANGLES, GX_VTXFMT0, mjWiiGL.numVertices);
      break;
    case GL_POINTS:
      GX_Begin(GX_POINTS, GX_VTXFMT0, mjWiiGL.numVertices);
      break;
    case GL_LINES:
      GX_Begin(GX_LINES, GX_VTXFMT0, mjWiiGL.numVertices);
      break;
    case GL_TRIANGLE_STRIP:
      GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, mjWiiGL.numVertices);
      break;
    case GL_TRIANGLE_FAN:
      GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, mjWiiGL.numVertices);
	  break;
    case GL_QUADS:
      GX_Begin(GX_QUADS, GX_VTXFMT0, mjWiiGL.numVertices);
      break;
    default:
      break;
    }
  
}

void glPushMatrix()
{
  if (mjWiiGL.index[mjWiiGL.currentMM] < mjWiiGL.max[mjWiiGL.currentMM]-1)
    {
      mjWiiGL.index[mjWiiGL.currentMM]++;
      guMtxCopy(*mjWiiGL.currentMtx, mjWiiGL.stacks[mjWiiGL.currentMM][mjWiiGL.index[mjWiiGL.currentMM]]);
      mjWiiGL.currentMtx = &mjWiiGL.stacks[mjWiiGL.currentMM][mjWiiGL.index[mjWiiGL.currentMM]];
  
    } else
    {
      //FIXME: Generate GLError
    }
}

void glPopMatrix()
{
  if (mjWiiGL.index[mjWiiGL.currentMM])
    {
      mjWiiGL.index[mjWiiGL.currentMM]--;
      mjWiiGL.currentMtx =  &mjWiiGL.stacks[mjWiiGL.currentMM][mjWiiGL.index[mjWiiGL.currentMM]];
    } else
    {
      //FIXME: Generate GLError
    }
}

void glLoadIdentity()
{
  guMtxIdentity(* mjWiiGL.currentMtx);
}

void gluLookAt(GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
	       GLfloat centerX, GLfloat centerY, GLfloat centerZ,
	       GLfloat upX, GLfloat upY, GLfloat upZ)
{
  guVector eye, center, up;
  eye.x = eyeX;
  eye.y = eyeY;
  eye.z = eyeZ;
  
  center.x = centerX;
  center.y = centerY;
  center.z = centerZ;
  
  up.x = upX;
  up.y = upY;
  up.z = upZ;
  
  guLookAt(* mjWiiGL.currentMtx, &eye, &up, &center);

}

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat alpha)
{
  GXColor bg;
  bg.r = (r*128);
  bg.g = (g*128);
  bg.b = (b*128);
  bg.a = (alpha*128);
  GX_SetCopyClear(bg, mjWiiGL.SCCZvalue);


}

void glVertex3f(float x, float y, float z)
{
	GX_Position3f32(x,y,z);
	GX_Color3f32(mjWiiCurrentColor.x, mjWiiCurrentColor.y, mjWiiCurrentColor.z);
}

void glColor3f(float x, float y, float z)
{
	mjWiiCurrentColor.x = x;
	mjWiiCurrentColor.y = y;
	mjWiiCurrentColor.z = z;
}
