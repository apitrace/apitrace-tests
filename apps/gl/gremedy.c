/*
 * Copyright (c) 1991, 1992, 1993 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF
 * ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */


#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif


static int win;


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


static void checkGlError(const char *call) {
   GLenum error = glGetError();
   if (error != GL_NO_ERROR) {
      fprintf(stderr, "error: %s -> %s\n", call, gluErrorString(error));
      exit(1);
   }
}

static void Init(void)
{
   const GLubyte *extensions;
   GLboolean has_GL_GREMEDY_string_marker;
   GLboolean has_GL_GREMEDY_frame_terminator;
      
   extensions = glGetString(GL_EXTENSIONS);
   checkGlError("glGetString(GL_EXTENSIONS)");
   has_GL_GREMEDY_string_marker = checkExtension("GL_GREMEDY_string_marker", extensions);
   has_GL_GREMEDY_frame_terminator = checkExtension("GL_GREMEDY_string_marker", extensions);

   if (GLEW_VERSION_3_0) {
      GLint has_GL3_GREMEDY_string_marker = 0;
      GLint has_GL3_GREMEDY_frame_terminator = 0;
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

         if (strcmp(extension, "GL_GREMEDY_string_marker_count") == 0) {
            ++has_GL3_GREMEDY_string_marker;
         }

         if (strcmp(extension, "GL_GREMEDY_frame_terminator") == 0) {
            ++has_GL3_GREMEDY_frame_terminator;
         }
      }

      if (!!has_GL_GREMEDY_string_marker != !!has_GL3_GREMEDY_string_marker) {
         fprintf(stderr, "error: GL_GREMEDY_string_marker not consistently supported by GL3\n");
         exit(1);
      }

      if (!!has_GL_GREMEDY_frame_terminator != !!has_GL3_GREMEDY_frame_terminator) {
         fprintf(stderr, "error: GL_GREMEDY_frame_terminator not consistently supported by GL3\n");
         exit(1);
      }
   }

   glewInit();

   if (!!has_GL_GREMEDY_string_marker != !!GLEW_GREMEDY_string_marker) {
      fprintf(stderr, "error: GL_GREMEDY_string_marker not consistently supported by GLEW\n");
      exit(1);
   }

   if (!!has_GL_GREMEDY_frame_terminator != !!GLEW_GREMEDY_frame_terminator) {
      fprintf(stderr, "error: GL_GREMEDY_frame_terminator not consistently supported by GLEW\n");
      exit(1);
   }

   if (GLEW_GREMEDY_string_marker) {
      glStringMarkerGREMEDY(strlen("Init"), "Init - this should not be included");
   }

   glClearColor(0.3, 0.1, 0.3, 1.0);
}

static void Reshape(int width, int height)
{
   if (GLEW_GREMEDY_string_marker) {
      glStringMarkerGREMEDY(0, __FUNCTION__);
   }

   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
   glMatrixMode(GL_MODELVIEW);
}

static void Draw(void)
{
   if (GLEW_GREMEDY_string_marker) {
      glStringMarkerGREMEDY(0, __FUNCTION__);
   }

   glClear(GL_COLOR_BUFFER_BIT); 

   glBegin(GL_TRIANGLES);
   glColor3f(.8,0,0); 
   glVertex3f(-0.9, -0.9, -30.0);
   glColor3f(0,.9,0); 
   glVertex3f( 0.9, -0.9, -30.0);
   glColor3f(0,0,.7); 
   glVertex3f( 0.0,  0.9, -30.0);
   glEnd();

   glFlush();

   if (GLEW_GREMEDY_frame_terminator) {
      glFrameTerminatorGREMEDY();
   }

   glutDestroyWindow(win);
   
   exit(0);
}

int main(int argc, char **argv)
{
   glutInit(&argc, argv);

   glutInitWindowPosition(0, 0);
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
