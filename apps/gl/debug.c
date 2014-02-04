/**************************************************************************
 *
 * Copyright 2013 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"


static int win;


enum DebugExtension {
    KHR_DEBUG,
    ARB_DEBUG_OUTPUT,
    AMD_DEBUG_OUTPUT,
    EXT_DEBUG_MARKER
};

static enum DebugExtension debugExtension = ARB_DEBUG_OUTPUT;


static const char *debugExtensionString = "GL_ARB_debug_output";


static void
parseArgs(int argc, char** argv)
{
    int i;

    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "GL_KHR_debug") == 0) {
#ifndef GL_KHR_debug
#  warning GL_KHR_debug not supported by this version of GLEW
            fprintf(stderr, "error: GL_KHR_debug not supported by this version of GLEW\n");
            exit(1);
#else
            debugExtension = KHR_DEBUG;
#endif
        } else if (strcmp(arg, "GL_ARB_debug_output") == 0) {
            debugExtension = ARB_DEBUG_OUTPUT;
        } else if (strcmp(arg, "GL_AMD_debug_output") == 0) {
            debugExtension = AMD_DEBUG_OUTPUT;
        } else if (strcmp(arg, "GL_EXT_debug_marker") == 0) {
            debugExtension = EXT_DEBUG_MARKER;
        } else {
            fprintf(stderr, "error: unknown extension %s\n", arg);
            exit(1);
        }
        debugExtensionString = arg;
    }
}


typedef void (GLAPIENTRY * PFNDEBUGMESSAGEINSERT)(GLsizei length, const GLchar *buf);
typedef void (GLAPIENTRY * PFNPUSHDEBUGGROUP)(GLsizei length, const char *message);
typedef void (GLAPIENTRY * PFNPOPDEBUGGROUP)(void);

static void GLAPIENTRY noopDebugMessageInsert(GLsizei length, const GLchar *buf) {}
static void GLAPIENTRY noopPushDebugGroup(GLsizei length, const char *message) {}
static void GLAPIENTRY noopPopDebugGroup(void) {}

static void
khrDebugMessageInsert(GLsizei length, const GLchar *buf) {
#ifdef GL_KHR_debug
   glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_MEDIUM, length, buf);
#endif
}

static void
arbDebugMessageInsert(GLsizei length, const GLchar *buf) {
   glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_MEDIUM_ARB, length, buf);
}

static void
amdDebugMessageInsert(GLsizei length, const GLchar *buf) {
   glDebugMessageInsertAMD(GL_DEBUG_CATEGORY_APPLICATION_AMD, GL_DEBUG_SEVERITY_MEDIUM_AMD, 0, length, buf);
}

static void
extDebugMessageInsert(GLsizei length, const GLchar *buf) {
   if (length < 0) length = 0;
   glInsertEventMarkerEXT(length, buf);
}

static void
khrPushDebugGroup(GLsizei length, const char *message) {
#ifdef GL_KHR_debug
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION_ARB, 0, length, message);
#endif
}

static void
khrPopDebugGroup(void) {
#ifdef GL_KHR_debug
    glPopDebugGroup();
#endif
}

static void
extPushDebugGroup(GLsizei length, const char *message) {
   if (length < 0) length = 0;
   glPushGroupMarkerEXT(length, message);
}

static void
extPopDebugGroup(void) {
    glPopGroupMarkerEXT();
}

static PFNDEBUGMESSAGEINSERT debugMessageInsert = noopDebugMessageInsert;
static PFNPUSHDEBUGGROUP pushDebugGroup = noopPushDebugGroup;
static PFNPOPDEBUGGROUP popDebugGroup = noopPopDebugGroup;


static void
Init(void)
{
    const GLubyte *extensions;
    GLboolean hasDebugExtension;
       
    extensions = glGetString(GL_EXTENSIONS);
    checkGlError("glGetString(GL_EXTENSIONS)");
    hasDebugExtension = checkExtension(debugExtensionString, extensions);

    if (GLEW_VERSION_3_0) {
       GLboolean hasDebugExtension3 = GL_FALSE;
       GLint i;

       GLint num_extensions = 0;
       glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
       checkGlError("glGetIntegerv(GL_NUM_EXTENSIONS)");

       for (i = 0; i < num_extensions; ++i) {
          const char *extension;

          extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
          checkGlError("glGetStringi(GL_EXTENSIONS, i)");

          if (strlen(extension) == 0) {
             fprintf(stderr, "error: glGetStringi returned empty string\n");
             exit(1);
          }

          if (strcmp(extension, debugExtensionString) == 0) {
             hasDebugExtension3 = GL_TRUE;
          }
       }

       if (hasDebugExtension != hasDebugExtension3) {
          fprintf(stderr, "error: %s not consistently supported by GL3\n", debugExtensionString);
          exit(1);
       }
    }

    glewInit();

    if (hasDebugExtension != glewIsSupported(debugExtensionString)) {
       fprintf(stderr, "error: %s not consistently supported by GLEW\n", debugExtensionString);
       exit(1);
    }

    if (hasDebugExtension) {
       switch (debugExtension) {
       case KHR_DEBUG:
#ifdef GL_KHR_debug
           debugMessageInsert = khrDebugMessageInsert;
           pushDebugGroup = khrPushDebugGroup;
           popDebugGroup = khrPopDebugGroup;
#endif
           break;
       case ARB_DEBUG_OUTPUT:
           debugMessageInsert = arbDebugMessageInsert;
           break;
       case AMD_DEBUG_OUTPUT:
           debugMessageInsert = amdDebugMessageInsert;
           break;
       case EXT_DEBUG_MARKER:
           debugMessageInsert = extDebugMessageInsert;
           pushDebugGroup = extPushDebugGroup;
           popDebugGroup = extPopDebugGroup;
           break;
       }
    } else {
       fprintf(stderr, "warning: %s not supported\n", debugExtensionString);
    }

    debugMessageInsert(-1, __FUNCTION__);
    pushDebugGroup(-1, __FUNCTION__);

    glClearColor(0.3, 0.1, 0.3, 1.0);
    
    popDebugGroup();
}

static void Reshape(int width, int height)
{
    debugMessageInsert(strlen("Reshape"), "Reshape" "-- this should not be included");
    pushDebugGroup(strlen("Reshape"), "Reshape" "-- this should not be included");

    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    popDebugGroup();
}

static void Draw(void)
{
    debugMessageInsert(-1, __FUNCTION__);
    pushDebugGroup(-1, __FUNCTION__);

    pushDebugGroup(-1, "Clear");
    glClear(GL_COLOR_BUFFER_BIT); 
    popDebugGroup();

    pushDebugGroup(-1, "Triangle");
    glBegin(GL_TRIANGLES);
    glColor3f(.8,0,0); 
    glVertex3f(-0.9, -0.9, -30.0);
    glColor3f(0,.9,0); 
    glVertex3f( 0.9, -0.9, -30.0);
    glColor3f(0,0,.7); 
    glVertex3f( 0.0,  0.9, -30.0);
    glEnd();
    popDebugGroup();

    popDebugGroup();

    glFlush();

    glutDestroyWindow(win);
    
    exit(0);
}


int
main(int argc, char **argv)
{
    parseArgs(argc, argv);

    glutInit(&argc, argv);

    glutInitWindowSize(250, 250);

    glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_SINGLE);

    win = glutCreateWindow(*argv);
    if (!win) {
        exit(1);
    }

    Init();

    glutReshapeFunc(Reshape);
    glutDisplayFunc(Draw);
    glutMainLoop();
    return 0;
}
