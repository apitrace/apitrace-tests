/*
 * Copyright (C) 2008  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLES/gl.h>  /* use OpenGL ES 1.x */
#include <GLES/glext.h>
#include <EGL/egl.h>

#include "eglut.h"


typedef void (GL_APIENTRY *PFNGLINSERTEVENTMARKEREXT)(GLsizei length, const char *marker);
typedef void (GL_APIENTRY *PFNGLPUSHGROUPMARKEREXT)(GLsizei length, const char *marker);
typedef void (GL_APIENTRY *PFNGLPOPGROUPMARKEREXT)(void);

static PFNGLINSERTEVENTMARKEREXT glInsertEventMarkerEXT = NULL;
static PFNGLPUSHGROUPMARKEREXT glPushGroupMarkerEXT = NULL;
static PFNGLPOPGROUPMARKEREXT glPopGroupMarkerEXT = NULL;
   
GLboolean has_GL_EXT_debug_marker = GL_FALSE;


/**
 * Identical to gluCheckExtension, which is not part of GLU on Windows.
 */
static GLboolean
checkExtension(const char *extName, const GLubyte *extString)
{
   const char *p = (const char *)extString;
   const char *q = extName;
   char c;
   do {
       c = *p++;
       if (c == '\0' || c == ' ') {
           if (q && *q == '\0') {
               return GL_TRUE;
           } else {
               q = extName;
           }
       } else {
           if (q && *q == c) {
               ++q;
           } else {
               q = 0;
           }
       }
   } while (c);
   return GL_FALSE;
}


static void
checkGlError(const char *call)
{
   GLenum error = glGetError();
   if (error != GL_NO_ERROR) {
      fprintf(stderr, "error: %s -> 0x%04x\n", call, error);
      exit(1);
   }
}


static void
idle(void)
{
   exit(0);
}


static void
draw(void)
{
   if (has_GL_EXT_debug_marker) {
       glPushGroupMarkerEXT(0, __FUNCTION__);
   }

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (has_GL_EXT_debug_marker) {
       glPopGroupMarkerEXT();
   }

   eglutIdleFunc(idle);
}


/* new window size or exposure */
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);
}


static void
init(void)
{
   const GLubyte *extensions;
      
   extensions = glGetString(GL_EXTENSIONS);
   checkGlError("glGetString(GL_EXTENSIONS)");
   has_GL_EXT_debug_marker = checkExtension("GL_EXT_debug_marker", extensions);

   if (has_GL_EXT_debug_marker) {

#define GET_PROC(name, type) \
   name = (type)eglGetProcAddress(#name); \
   if (!name) { \
      fprintf(stderr, "error: eglGetProcAddress(\"" #name "\" returned NULL\n"); \
      exit(1); \
   }

      GET_PROC(glInsertEventMarkerEXT, PFNGLINSERTEVENTMARKEREXT)
      GET_PROC(glPushGroupMarkerEXT, PFNGLPUSHGROUPMARKEREXT)
      GET_PROC(glPopGroupMarkerEXT, PFNGLPOPGROUPMARKEREXT)

#undef GET_PROC

      glInsertEventMarkerEXT(strlen("Init"), "Init - this should not be included");
   }

   glClearColor(0.4, 0.4, 0.4, 0.0);
}

int
main(int argc, char *argv[])
{
   eglutInitWindowSize(300, 300);
   eglutInitAPIMask(EGLUT_OPENGL_ES1_BIT);
   eglutInit(argc, argv);

   eglutCreateWindow("debug_marker");

   eglutReshapeFunc(reshape);
   eglutDisplayFunc(draw);

   init();

   eglutMainLoop();

   return 0;
}
