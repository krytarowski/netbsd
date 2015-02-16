/* $XFree86: xc/lib/GL/glx/single2.c,v 1.16 2004/12/10 16:06:56 alanh Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#define NEED_GL_FUNCS_WRAPPED
#include <stdio.h>
#include "glxclient.h"
#include "packsingle.h"
#include "glxextensions.h"

/* Used for GL_ARB_transpose_matrix */
static void TransposeMatrixf(GLfloat m[16])
{
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            GLfloat tmp = m[i*4+j];
            m[i*4+j] = m[j*4+i];
            m[j*4+i] = tmp;
        }
    }
}

/* Used for GL_ARB_transpose_matrix */
static void TransposeMatrixb(GLboolean m[16])
{
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            GLboolean tmp = m[i*4+j];
            m[i*4+j] = m[j*4+i];
            m[j*4+i] = tmp;
        }
    }
}

/* Used for GL_ARB_transpose_matrix */
static void TransposeMatrixd(GLdouble m[16])
{
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            GLdouble tmp = m[i*4+j];
            m[i*4+j] = m[j*4+i];
            m[j*4+i] = tmp;
        }
    }
}

/* Used for GL_ARB_transpose_matrix */
static void TransposeMatrixi(GLint m[16])
{
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            GLint tmp = m[i*4+j];
            m[i*4+j] = m[j*4+i];
            m[j*4+i] = tmp;
        }
    }
}

GLenum glGetError(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    GLuint retval = GL_NO_ERROR;
    xGLXGetErrorReply reply;

    if (gc->error) {
	/* Use internal error first */
	retval = gc->error;
	gc->error = GL_NO_ERROR;
	return retval;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetError,0);
    __GLX_SINGLE_READ_XREPLY();
    retval = reply.error;
    __GLX_SINGLE_END();

    return retval;
}

void glGetClipPlane(GLenum plane, GLdouble *equation)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;
    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetClipPlane,4);
    __GLX_SINGLE_PUT_LONG(0,plane);
    __GLX_SINGLE_READ_XREPLY();
    if (reply.length == 8) {
	__GLX_SINGLE_GET_DOUBLE_ARRAY(equation,4);
    }
    __GLX_SINGLE_END();
}

#define CASE_ARRAY_ENABLE(enum_name,array,dest,gl_type) \
    case GL_ ## enum_name ## _ARRAY: \
      *dest = (gl_type) (IS_ARRAY_ENABLED(state, array)); break
#define CASE_ARRAY_SIZE(enum_name,array,dest,gl_type) \
    case GL_ ## enum_name ## _ARRAY_SIZE: \
      *dest = (gl_type) state->vertArray.arrays[array ## _ARRAY].size ; break
#define CASE_ARRAY_TYPE(enum_name,array,dest,gl_type) \
    case GL_ ## enum_name ## _ARRAY_TYPE: \
      *dest = (gl_type) state->vertArray.arrays[array ## _ARRAY].type ; break
#define CASE_ARRAY_STRIDE(enum_name,array,dest,gl_type) \
    case GL_ ## enum_name ## _ARRAY_STRIDE: \
      *dest = (gl_type) state->vertArray.arrays[array ## _ARRAY].stride ; break

#define CASE_ARRAY_ALL(enum_name,array,dest,gl_type) \
	CASE_ARRAY_ENABLE(enum_name,array,dest,gl_type); \
	CASE_ARRAY_STRIDE(enum_name,array,dest,gl_type); \
	CASE_ARRAY_TYPE(enum_name,array,dest,gl_type); \
	CASE_ARRAY_SIZE(enum_name,array,dest,gl_type)

void glGetBooleanv(GLenum val, GLboolean *b)
{
    const GLenum origVal = val;
    __GLX_SINGLE_DECLARE_VARIABLES();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    xGLXSingleReply reply;

    if (val == GL_TRANSPOSE_MODELVIEW_MATRIX_ARB) {
       val = GL_MODELVIEW_MATRIX;
    }
    else if (val == GL_TRANSPOSE_PROJECTION_MATRIX_ARB) {
       val = GL_PROJECTION_MATRIX;
    }
    else if (val == GL_TRANSPOSE_TEXTURE_MATRIX_ARB) {
       val = GL_TEXTURE_MATRIX;
    }
    else if (val == GL_TRANSPOSE_COLOR_MATRIX_ARB) {
       val = GL_COLOR_MATRIX;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetBooleanv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *b = (GLboolean)state->storePack.rowLength;
	    break;
	  case GL_PACK_IMAGE_HEIGHT:
	    *b = (GLboolean)state->storePack.imageHeight;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *b = (GLboolean)state->storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *b = (GLboolean)state->storePack.skipPixels;
	    break;
	  case GL_PACK_SKIP_IMAGES:
	    *b = (GLboolean)state->storePack.skipImages;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *b = (GLboolean)state->storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *b = (GLboolean)state->storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *b = (GLboolean)state->storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *b = (GLboolean)state->storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_IMAGE_HEIGHT:
	    *b = (GLboolean)state->storeUnpack.imageHeight;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *b = (GLboolean)state->storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *b = (GLboolean)state->storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_SKIP_IMAGES:
	    *b = (GLboolean)state->storeUnpack.skipImages;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *b = (GLboolean)state->storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *b = (GLboolean)state->storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *b = (GLboolean)state->storeUnpack.lsbFirst;
	    break;

	  CASE_ARRAY_ALL(VERTEX, vertex, b, GLboolean);

	  CASE_ARRAY_ENABLE(NORMAL, normal, b, GLboolean);
	  CASE_ARRAY_TYPE(NORMAL, normal, b, GLboolean);
	  CASE_ARRAY_STRIDE(NORMAL, normal, b, GLboolean);

	  CASE_ARRAY_ALL(COLOR, color, b, GLboolean);

	  CASE_ARRAY_ENABLE(INDEX, index, b, GLboolean);
	  CASE_ARRAY_TYPE(INDEX, index, b, GLboolean);
	  CASE_ARRAY_STRIDE(INDEX, index, b, GLboolean);

	  case GL_TEXTURE_COORD_ARRAY:
	    *b = (GLboolean)IS_TEXARRAY_ENABLED(state, state->vertArray.activeTexture);
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *b = (GLboolean)state->vertArray.texCoord[state->vertArray.activeTexture].size;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *b = (GLboolean)state->vertArray.texCoord[state->vertArray.activeTexture].type;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *b = (GLboolean)state->vertArray.texCoord[state->vertArray.activeTexture].stride;
	    break;

	  CASE_ARRAY_ENABLE(EDGE_FLAG, edgeFlag, b, GLboolean);
	  CASE_ARRAY_STRIDE(EDGE_FLAG, edgeFlag, b, GLboolean);

	  CASE_ARRAY_ALL(SECONDARY_COLOR, secondaryColor, b, GLboolean);

	  CASE_ARRAY_ENABLE(FOG_COORD, fogCoord, b, GLboolean);
	  CASE_ARRAY_TYPE(FOG_COORD, fogCoord, b, GLboolean);
	  CASE_ARRAY_STRIDE(FOG_COORD, fogCoord, b, GLboolean);

	  case GL_MAX_ELEMENTS_VERTICES:
	    *b = (GLboolean)state->vertArray.maxElementsVertices;
	    break;
	  case GL_MAX_ELEMENTS_INDICES:
	    *b = (GLboolean)state->vertArray.maxElementsIndices;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *b = (GLboolean)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  case GL_CLIENT_ACTIVE_TEXTURE_ARB:
	    *b = (GLboolean)(state->vertArray.activeTexture + GL_TEXTURE0_ARB);
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_CHAR(b);
	    } else {
		__GLX_SINGLE_GET_CHAR_ARRAY(b,compsize);
                if (val != origVal) {
                   /* matrix transpose */
                   TransposeMatrixb(b);
                }
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetDoublev(GLenum val, GLdouble *d)
{
    const GLenum origVal = val;
    __GLX_SINGLE_DECLARE_VARIABLES();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    xGLXSingleReply reply;

    if (val == GL_TRANSPOSE_MODELVIEW_MATRIX_ARB) {
       val = GL_MODELVIEW_MATRIX;
    }
    else if (val == GL_TRANSPOSE_PROJECTION_MATRIX_ARB) {
       val = GL_PROJECTION_MATRIX;
    }
    else if (val == GL_TRANSPOSE_TEXTURE_MATRIX_ARB) {
       val = GL_TEXTURE_MATRIX;
    }
    else if (val == GL_TRANSPOSE_COLOR_MATRIX_ARB) {
       val = GL_COLOR_MATRIX;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetDoublev,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *d = (GLdouble)state->storePack.rowLength;
	    break;
	  case GL_PACK_IMAGE_HEIGHT:
	    *d = (GLdouble)state->storePack.imageHeight;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *d = (GLdouble)state->storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *d = (GLdouble)state->storePack.skipPixels;
	    break;
	  case GL_PACK_SKIP_IMAGES:
	    *d = (GLdouble)state->storePack.skipImages;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *d = (GLdouble)state->storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *d = (GLdouble)state->storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *d = (GLdouble)state->storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *d = (GLdouble)state->storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_IMAGE_HEIGHT:
	    *d = (GLdouble)state->storeUnpack.imageHeight;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *d = (GLdouble)state->storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *d = (GLdouble)state->storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_SKIP_IMAGES:
	    *d = (GLdouble)state->storeUnpack.skipImages;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *d = (GLdouble)state->storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *d = (GLdouble)state->storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *d = (GLdouble)state->storeUnpack.lsbFirst;
	    break;

	  CASE_ARRAY_ALL(VERTEX, vertex, d, GLdouble);

	  CASE_ARRAY_ENABLE(NORMAL, normal, d, GLdouble);
	  CASE_ARRAY_TYPE(NORMAL, normal, d, GLdouble);
	  CASE_ARRAY_STRIDE(NORMAL, normal, d, GLdouble);

	  CASE_ARRAY_ALL(COLOR, color, d, GLdouble);

	  CASE_ARRAY_ENABLE(INDEX, index, d, GLdouble);
	  CASE_ARRAY_TYPE(INDEX, index, d, GLdouble);
	  CASE_ARRAY_STRIDE(INDEX, index, d, GLdouble);

	  case GL_TEXTURE_COORD_ARRAY:
	    *d = (GLdouble) IS_TEXARRAY_ENABLED(state, state->vertArray.activeTexture);
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *d = (GLdouble)state->vertArray.texCoord[state->vertArray.activeTexture].size;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *d = (GLdouble)state->vertArray.texCoord[state->vertArray.activeTexture].type;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *d = (GLdouble)state->vertArray.texCoord[state->vertArray.activeTexture].stride;
	    break;

	  CASE_ARRAY_ENABLE(EDGE_FLAG, edgeFlag, d, GLdouble);
	  CASE_ARRAY_STRIDE(EDGE_FLAG, edgeFlag, d, GLdouble);

	  CASE_ARRAY_ALL(SECONDARY_COLOR, secondaryColor, d, GLdouble);

	  CASE_ARRAY_ENABLE(FOG_COORD, fogCoord, d, GLdouble);
	  CASE_ARRAY_TYPE(FOG_COORD, fogCoord, d, GLdouble);
	  CASE_ARRAY_STRIDE(FOG_COORD, fogCoord, d, GLdouble);

	  case GL_MAX_ELEMENTS_VERTICES:
	    *d = (GLdouble)state->vertArray.maxElementsVertices;
	    break;
	  case GL_MAX_ELEMENTS_INDICES:
	    *d = (GLdouble)state->vertArray.maxElementsIndices;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *d = (GLdouble)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  case GL_CLIENT_ACTIVE_TEXTURE_ARB:
	    *d = (GLdouble)(state->vertArray.activeTexture + GL_TEXTURE0_ARB);
	    break;
	  default:
	    /*
	     ** Not a local value, so use what we got from the server.
	     */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_DOUBLE(d);
	    } else {
		__GLX_SINGLE_GET_DOUBLE_ARRAY(d,compsize);
                if (val != origVal) {
                   /* matrix transpose */
                   TransposeMatrixd(d);
                }
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetFloatv(GLenum val, GLfloat *f)
{
    const GLenum origVal = val;
    __GLX_SINGLE_DECLARE_VARIABLES();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    xGLXSingleReply reply;

    if (val == GL_TRANSPOSE_MODELVIEW_MATRIX_ARB) {
       val = GL_MODELVIEW_MATRIX;
    }
    else if (val == GL_TRANSPOSE_PROJECTION_MATRIX_ARB) {
       val = GL_PROJECTION_MATRIX;
    }
    else if (val == GL_TRANSPOSE_TEXTURE_MATRIX_ARB) {
       val = GL_TEXTURE_MATRIX;
    }
    else if (val == GL_TRANSPOSE_COLOR_MATRIX_ARB) {
       val = GL_COLOR_MATRIX;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetFloatv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *f = (GLfloat)state->storePack.rowLength;
	    break;
	  case GL_PACK_IMAGE_HEIGHT:
	    *f = (GLfloat)state->storePack.imageHeight;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *f = (GLfloat)state->storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *f = (GLfloat)state->storePack.skipPixels;
	    break;
	  case GL_PACK_SKIP_IMAGES:
	    *f = (GLfloat)state->storePack.skipImages;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *f = (GLfloat)state->storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *f = (GLfloat)state->storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *f = (GLfloat)state->storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *f = (GLfloat)state->storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_IMAGE_HEIGHT:
	    *f = (GLfloat)state->storeUnpack.imageHeight;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *f = (GLfloat)state->storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *f = (GLfloat)state->storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_SKIP_IMAGES:
	    *f = (GLfloat)state->storeUnpack.skipImages;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *f = (GLfloat)state->storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *f = (GLfloat)state->storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *f = (GLfloat)state->storeUnpack.lsbFirst;
	    break;

	  CASE_ARRAY_ALL(VERTEX, vertex, f, GLfloat);

	  CASE_ARRAY_ENABLE(NORMAL, normal, f, GLfloat);
	  CASE_ARRAY_TYPE(NORMAL, normal, f, GLfloat);
	  CASE_ARRAY_STRIDE(NORMAL, normal, f, GLfloat);

	  CASE_ARRAY_ALL(COLOR, color, f, GLfloat);

	  CASE_ARRAY_ENABLE(INDEX, index, f, GLfloat);
	  CASE_ARRAY_TYPE(INDEX, index, f, GLfloat);
	  CASE_ARRAY_STRIDE(INDEX, index, f, GLfloat);

	  case GL_TEXTURE_COORD_ARRAY:
	    *f = (GLfloat) IS_TEXARRAY_ENABLED(state, state->vertArray.activeTexture);
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *f = (GLfloat)state->vertArray.texCoord[state->vertArray.activeTexture].size;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *f = (GLfloat)state->vertArray.texCoord[state->vertArray.activeTexture].type;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *f = (GLfloat)state->vertArray.texCoord[state->vertArray.activeTexture].stride;
	    break;

	  CASE_ARRAY_ENABLE(EDGE_FLAG, edgeFlag, f, GLfloat);
	  CASE_ARRAY_STRIDE(EDGE_FLAG, edgeFlag, f, GLfloat);

	  CASE_ARRAY_ALL(SECONDARY_COLOR, secondaryColor, f, GLfloat);

	  CASE_ARRAY_ENABLE(FOG_COORD, fogCoord, f, GLfloat);
	  CASE_ARRAY_TYPE(FOG_COORD, fogCoord, f, GLfloat);
	  CASE_ARRAY_STRIDE(FOG_COORD, fogCoord, f, GLfloat);

	  case GL_MAX_ELEMENTS_VERTICES:
	    *f = (GLfloat)state->vertArray.maxElementsVertices;
	    break;
	  case GL_MAX_ELEMENTS_INDICES:
	    *f = (GLfloat)state->vertArray.maxElementsIndices;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *f = (GLfloat)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  case GL_CLIENT_ACTIVE_TEXTURE_ARB:
	    *f = (GLfloat)(state->vertArray.activeTexture + GL_TEXTURE0_ARB);
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_FLOAT(f);
	    } else {
		__GLX_SINGLE_GET_FLOAT_ARRAY(f,compsize);
                if (val != origVal) {
                   /* matrix transpose */
                   TransposeMatrixf(f);
                }
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetIntegerv(GLenum val, GLint *i)
{
    const GLenum origVal = val;
    __GLX_SINGLE_DECLARE_VARIABLES();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    xGLXSingleReply reply;

    if (val == GL_TRANSPOSE_MODELVIEW_MATRIX_ARB) {
       val = GL_MODELVIEW_MATRIX;
    }
    else if (val == GL_TRANSPOSE_PROJECTION_MATRIX_ARB) {
       val = GL_PROJECTION_MATRIX;
    }
    else if (val == GL_TRANSPOSE_TEXTURE_MATRIX_ARB) {
       val = GL_TEXTURE_MATRIX;
    }
    else if (val == GL_TRANSPOSE_COLOR_MATRIX_ARB) {
       val = GL_COLOR_MATRIX;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetIntegerv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *i = (GLint)state->storePack.rowLength;
	    break;
	  case GL_PACK_IMAGE_HEIGHT:
	    *i = (GLint)state->storePack.imageHeight;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *i = (GLint)state->storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *i = (GLint)state->storePack.skipPixels;
	    break;
	  case GL_PACK_SKIP_IMAGES:
	    *i = (GLint)state->storePack.skipImages;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *i = (GLint)state->storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *i = (GLint)state->storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *i = (GLint)state->storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *i = (GLint)state->storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_IMAGE_HEIGHT:
	    *i = (GLint)state->storeUnpack.imageHeight;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *i = (GLint)state->storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *i = (GLint)state->storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_SKIP_IMAGES:
	    *i = (GLint)state->storeUnpack.skipImages;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *i = (GLint)state->storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *i = (GLint)state->storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *i = (GLint)state->storeUnpack.lsbFirst;
	    break;

	  CASE_ARRAY_ALL(VERTEX, vertex, i, GLint);

	  CASE_ARRAY_ENABLE(NORMAL, normal, i, GLint);
	  CASE_ARRAY_TYPE(NORMAL, normal, i, GLint);
	  CASE_ARRAY_STRIDE(NORMAL, normal, i, GLint);

	  CASE_ARRAY_ALL(COLOR, color, i, GLint);

	  CASE_ARRAY_ENABLE(INDEX, index, i, GLint);
	  CASE_ARRAY_TYPE(INDEX, index, i, GLint);
	  CASE_ARRAY_STRIDE(INDEX, index, i, GLint);

	  case GL_TEXTURE_COORD_ARRAY:
	    *i = (GLint) IS_TEXARRAY_ENABLED(state, state->vertArray.activeTexture);
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *i = (GLint)state->vertArray.texCoord[state->vertArray.activeTexture].size;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *i = (GLint)state->vertArray.texCoord[state->vertArray.activeTexture].type;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *i = (GLint)state->vertArray.texCoord[state->vertArray.activeTexture].stride;
	    break;

	  CASE_ARRAY_ENABLE(EDGE_FLAG, edgeFlag, i, GLint);
	  CASE_ARRAY_STRIDE(EDGE_FLAG, edgeFlag, i, GLint);

	  CASE_ARRAY_ALL(SECONDARY_COLOR, secondaryColor, i, GLint);

	  CASE_ARRAY_ENABLE(FOG_COORD, fogCoord, i, GLint);
	  CASE_ARRAY_TYPE(FOG_COORD, fogCoord, i, GLint);
	  CASE_ARRAY_STRIDE(FOG_COORD, fogCoord, i, GLint);

	  case GL_MAX_ELEMENTS_VERTICES:
	    *i = (GLint)state->vertArray.maxElementsVertices;
	    break;
	  case GL_MAX_ELEMENTS_INDICES:
	    *i = (GLint)state->vertArray.maxElementsIndices;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *i = (GLint)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  case GL_CLIENT_ACTIVE_TEXTURE_ARB:
	    *i = (GLint)(state->vertArray.activeTexture + GL_TEXTURE0_ARB);
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_LONG(i);
	    } else {
		__GLX_SINGLE_GET_LONG_ARRAY(i,compsize);
                if (val != origVal) {
                   /* matrix transpose */
                   TransposeMatrixi(i);
                }
	    }
	}
    }
    __GLX_SINGLE_END();
}

/*
** Send all pending commands to server.
*/
void glFlush(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_Flush,0);
    __GLX_SINGLE_END();

    /* And finally flush the X protocol data */
    XFlush(dpy);
}

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_FeedbackBuffer,8);
    __GLX_SINGLE_PUT_LONG(0,size);
    __GLX_SINGLE_PUT_LONG(4,type);
    __GLX_SINGLE_END();

    gc->feedbackBuf = buffer;
}

void glSelectBuffer(GLsizei numnames, GLuint *buffer)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_SelectBuffer,4);
    __GLX_SINGLE_PUT_LONG(0,numnames);
    __GLX_SINGLE_END();

    gc->selectBuf = buffer;
}

GLint glRenderMode(GLenum mode)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    GLint retval;
    xGLXRenderModeReply reply;

    if (!dpy) return -1;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_RenderMode,4);
    __GLX_SINGLE_PUT_LONG(0,mode);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_RETVAL(retval,GLint);

    if (reply.newMode != mode) {
	/*
	** Switch to new mode did not take effect, therefore an error
	** occured.  When an error happens the server won't send us any
	** other data.
	*/
    } else {
	/* Read the feedback or selection data */
	if (gc->renderMode == GL_FEEDBACK) {
	    __GLX_SINGLE_GET_SIZE(compsize);
	    __GLX_SINGLE_GET_FLOAT_ARRAY(gc->feedbackBuf, compsize);
	} else
	if (gc->renderMode == GL_SELECT) {
	    __GLX_SINGLE_GET_SIZE(compsize);
	    __GLX_SINGLE_GET_LONG_ARRAY(gc->selectBuf, compsize);
	}
	gc->renderMode = mode;
    }
    __GLX_SINGLE_END();

    return retval;
}

void glFinish(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_Finish,0);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_END();
}


/**
 * Extract the major and minor version numbers from a version string.
 */
static void
version_from_string( const char * ver, 
		     int * major_version, int * minor_version )
{
    const char * end;
    long major;
    long minor;

    major = strtol( ver, (char **) & end, 10 );
    minor = strtol( end + 1, NULL, 10 );
    *major_version = major;
    *minor_version = minor;
}


const GLubyte *glGetString(GLenum name)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;
    GLubyte *s = NULL;

    if (!dpy) return 0;

    /*
    ** Return the cached copy if the string has already been fetched
    */
    switch(name) {
      case GL_VENDOR:
	if (gc->vendor) return gc->vendor;
	break;
      case GL_RENDERER:
	if (gc->renderer) return gc->renderer;
	break;
      case GL_VERSION:
	if (gc->version) return gc->version;
	break;
      case GL_EXTENSIONS:
	if (gc->extensions) return gc->extensions;
	break;
      default:
	__glXSetError(gc, GL_INVALID_ENUM);
	return 0;
    }

    /*
    ** Get requested string from server
    */

    (void) __glXFlushRenderBuffer( gc, gc->pc );
    s = (GLubyte *) __glXGetStringFromServer( dpy, gc->majorOpcode,
				  X_GLsop_GetString, gc->currentContextTag,
				  name );
    if (!s) {
	/* Throw data on the floor */
	__glXSetError(gc, GL_OUT_OF_MEMORY);
    } else {
	/*
	** Update local cache
	*/
	switch(name) {
	case GL_VENDOR:
	    gc->vendor = s;
	    break;

	case GL_RENDERER:
	    gc->renderer = s;
	    break;

	case GL_VERSION: {
	    int client_major;
	    int client_minor;

	    version_from_string( (char *) s, 
				 & gc->server_major, & gc->server_minor );
	    __glXGetGLVersion( & client_major, & client_minor );

	    if ( (gc->server_major < client_major)
		 || ((gc->server_major == client_major) 
		     && (gc->server_minor <= client_minor)) ) {
		gc->version = s;
	    }
	    else {
		/* Allow 7 bytes for the client-side GL version.  This allows
		 * for upto version 999.999.  I'm not holding my breath for
		 * that one!  The extra 4 is for the ' ()\0' that will be
		 * added.
		 */
		const size_t size = 7 + strlen( (char *) s ) + 4;

		gc->version = Xmalloc( size );
		if ( gc->version == NULL ) {
		    /* If we couldn't allocate memory for the new string,
		     * make a best-effort and just copy the client-side version
		     * to the string and use that.  It probably doesn't
		     * matter what is done here.  If there not memory available
		     * for a short string, the system is probably going to die
		     * soon anyway.
		     */
		    snprintf( (char *) s, strlen( (char *) s ) + 1, "%u.%u",
			      client_major, client_minor );
		    gc->version = s;
		}
		else {
		    snprintf( (char *)gc->version, size, "%u.%u (%s)",
			      client_major, client_minor, s );
		    Xfree( s );
		    s = gc->version;
		}
	    }
	    break;
	}

	case GL_EXTENSIONS: {
	    int major = 1;
	    int minor = 0;

	    /* This code is currently disabled.  I was reminded that some
	     * vendors intentionally exclude some extensions from their
	     * extension string that are part of the core version they
	     * advertise.  In particular, on Nvidia drivers this means that
	     * the functionality is supported by the driver, but is not
	     * hardware accelerated.  For example, a TNT will show core
	     * version 1.5, but most of the post-1.2 functionality is a
	     * software fallback.
	     * 
	     * I don't want to break applications that rely on this odd
	     * behavior.  At the same time, the code is written and tested,
	     * so I didn't want to throw it away.  Therefore, the code is here
	     * but disabled.  In the future, we may wish to and an environment
	     * variable to enable it.
	     */
	    
#if 0
	    /* Call glGetString just to make sure that gc->server_major and
	     * gc->server_minor are set.  This version may be higher than we
	     * can completely support, but it may imply support for some
	     * extensions that we can support.
	     * 
	     * For example, at the time of this writing, the client-side
	     * library only supports upto core GL version 1.2.  However, cubic
	     * textures, multitexture, multisampling, and some other 1.3
	     * features are supported.  If the server reports back version
	     * 1.3, but does not report all of those extensions, we will
	     * enable them.
	     */
	    (void *) glGetString( GL_VERSION );
	    major = gc->server_major,
	    minor = gc->server_minor;
#endif

	    __glXCalculateUsableGLExtensions( gc, (char *) s, major, minor );
	    XFree( s );
	    s = gc->extensions;
	    break;
	}
	}
    }
    return s;
}

GLboolean glIsEnabled(GLenum cap)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    xGLXSingleReply reply;
    GLboolean retval = 0;

    if (!dpy) return 0;

    switch(cap) {
      case GL_VERTEX_ARRAY:
	  return IS_ARRAY_ENABLED(state, vertex);
      case GL_NORMAL_ARRAY:
	  return IS_ARRAY_ENABLED(state, normal);
      case GL_COLOR_ARRAY:
	  return IS_ARRAY_ENABLED(state, color);
      case GL_INDEX_ARRAY:
	  return IS_ARRAY_ENABLED(state, index);
      case GL_TEXTURE_COORD_ARRAY:
	  return IS_TEXARRAY_ENABLED(state, state->vertArray.activeTexture);
      case GL_EDGE_FLAG_ARRAY:
	  return IS_ARRAY_ENABLED(state, edgeFlag);
      case GL_SECONDARY_COLOR_ARRAY:
	  return IS_ARRAY_ENABLED(state, secondaryColor);
      case GL_FOG_COORD_ARRAY:
	  return IS_ARRAY_ENABLED(state, fogCoord);
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_IsEnabled,4);
    __GLX_SINGLE_PUT_LONG(0,cap);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_RETVAL(retval, GLboolean);
    __GLX_SINGLE_END();
    return retval;
}

void glGetPointerv(GLenum pname, void **params)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXattribute * state = (__GLXattribute *)(gc->client_state_private);
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    switch(pname) {
      case GL_VERTEX_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ vertex_ARRAY ].ptr;
	  return;
      case GL_NORMAL_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ normal_ARRAY ].ptr;
	  return;
      case GL_COLOR_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ color_ARRAY ].ptr;
	  return;
      case GL_INDEX_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ index_ARRAY ].ptr;
	  return;
      case GL_TEXTURE_COORD_ARRAY_POINTER:
	  *params = (void *)state->vertArray.texCoord[state->vertArray.activeTexture].ptr;
	  return;
      case GL_EDGE_FLAG_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ edgeFlag_ARRAY ].ptr;
	return;
      case GL_SECONDARY_COLOR_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ secondaryColor_ARRAY ].ptr;
	return;
      case GL_FOG_COORD_ARRAY_POINTER:
	  *params = (void *)state->vertArray.arrays[ fogCoord_ARRAY ].ptr;
	return;
      case GL_FEEDBACK_BUFFER_POINTER:
	*params = (void *)gc->feedbackBuf;
	return;
      case GL_SELECTION_BUFFER_POINTER:
	*params = (void *)gc->selectBuf;
	return;
      default:
	__glXSetError(gc, GL_INVALID_ENUM);
	return;
    }
}

