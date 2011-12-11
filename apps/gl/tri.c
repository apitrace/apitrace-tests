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

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


static GLboolean doubleBuffer = GL_TRUE;
static int win;

static void parseArgs(int argc, char** argv)
{
   int i;

   for (i = 1; i < argc; ++i) {
      const char *arg = argv[i];
      if (strcmp(arg, "-sb") == 0) {
         doubleBuffer = GL_FALSE;
      } else if (strcmp(arg, "-db") == 0) {
         doubleBuffer = GL_TRUE;
      } else {
         fprintf(stderr, "error: unknown arg %s\n", arg);
         exit(1);
      }
   }
}

static void Init(void)
{
   glClearColor(0.3, 0.1, 0.3, 1.0);
}

static void Reshape(int width, int height)
{

   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
   glMatrixMode(GL_MODELVIEW);
}

static void Draw(void)
{
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

   if (doubleBuffer) {
      glutSwapBuffers();
   }

   glutDestroyWindow(win);

   exit(0);
}

int main(int argc, char **argv)
{
   GLenum type;

   parseArgs(argc, argv);

   glutInit(&argc, argv);

   glutInitWindowSize(250, 250);

   type = GLUT_RGB | GLUT_ALPHA;
   type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
   glutInitDisplayMode(type);

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
