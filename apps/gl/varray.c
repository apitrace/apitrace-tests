/*
 * Copyright (c) 1993-1997, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
 */

/*
 *  varray.c
 *  This program demonstrates vertex arrays.
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>


static GLFWwindow* window = NULL;


enum SetupMethod {
   POINTER,
   INTERLEAVED,
};

enum DerefMethod {
   DRAWARRAYS,
   ARRAYELEMENT,
   DRAWELEMENTS,
};

static enum SetupMethod setupMethod = POINTER;
static enum DerefMethod derefMethod = DRAWELEMENTS;


static void parseArgs(int argc, char** argv)
{
   int i;

   for (i = 1; i < argc; ++i) {
      const char *arg = argv[i];
      if (strcmp(arg, "pointer") == 0) {
         setupMethod = POINTER;
      } else if (strcmp(arg, "interleaved") == 0) {
         setupMethod = INTERLEAVED;
      } else if (strcmp(arg, "drawarrays") == 0) {
         derefMethod = DRAWARRAYS;
      } else if (strcmp(arg, "arrayelement") == 0) {
         derefMethod = ARRAYELEMENT;
      } else if (strcmp(arg, "drawelements") == 0) {
         derefMethod = DRAWELEMENTS;
      } else {
         fprintf(stderr, "error: unknown arg %s\n", arg);
         exit(1);
      }
   }
}

static void setupPointers(void)
{
   static GLint vertices[] = {
        25,  25,
       100, 325,
       175,  25,
       175, 325,
       250,  25,
       325, 325
   };
   static GLfloat colors[] = {
      1.0 , 0.2 , 0.2 ,
      0.2 , 0.2 , 1.0 ,
      0.8 , 1.0 , 0.2 ,
      0.75, 0.75, 0.75,
      0.35, 0.35, 0.35,
      0.5 , 0.5 , 0.5
   };

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   glVertexPointer(2, GL_INT, 2 * sizeof(GLint), vertices);
   glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), colors);
}

static void setupInterleave(void)
{
   static GLfloat intertwined[] = {
       1.0 , 0.2 , 0.2 ,  25.0,  25.0, 0.0,
       0.2 , 0.2 , 1.0 , 100.0, 325.0, 0.0,
       0.8 , 1.0 , 0.2 , 175.0,  25.0, 0.0,
       0.75, 0.75, 0.75, 175.0, 325.0, 0.0,
       0.35, 0.35, 0.35, 250.0,  25.0, 0.0,
       0.5 , 0.5 , 0.5 , 325.0, 325.0, 0.0
   };

   glInterleavedArrays(GL_C3F_V3F, 0, intertwined);
}

static void init(void)
{
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glShadeModel(GL_SMOOTH);
   setupPointers();
}

static void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   if (derefMethod == DRAWARRAYS)
      glDrawArrays(GL_TRIANGLES, 0, 6);
   else if (derefMethod == ARRAYELEMENT) {
      glBegin(GL_TRIANGLES);
      glArrayElement(2);
      glArrayElement(3);
      glArrayElement(5);
      glEnd();
   }
   else if (derefMethod == DRAWELEMENTS) {
      GLuint indices[4] = {0, 1, 3, 4};

      glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, indices);
   }
   glFlush();

   glfwSwapBuffers(window);
}

static void reshape(void)
{
   int w, h;
   glfwGetFramebufferSize(window, &w, &h);

   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, (GLdouble) w, 0.0, (GLdouble) h, -1.0, 1.0);
}

int main(int argc, char** argv)
{
   parseArgs(argc, argv);

   glfwInit();

   window = glfwCreateWindow(350, 350, argv[0], NULL, NULL);
   if (!window) {
      exit(1);
   }

   glfwMakeContextCurrent(window);

   init();
   reshape();
   display();

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
