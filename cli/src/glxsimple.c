/**************************************************************************
 * Copyright 2012 Intel corporation
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

#include <X11/Xlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int width = 64;
int height = 64;

static void
paint_rgb_using_clear (double r, double g, double b)
{
        glClearColor(r, g, b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
}

static void
paint_rgb_using_glsl (double r, double g, double b)
{
	const char * vs_source =
		"void main()\n"
		"{\n"
		"        gl_Position = ftransform();\n"
		"}\n";
	const char * fs_source =
		"#version 120\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = color;\n"
		"}\n";

	GLuint vs, fs, program;
	GLint color;

	vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vs_source, NULL);
	glCompileShader (vs);

	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fs_source, NULL);
	glCompileShader (fs);

	program = glCreateProgram ();
	glAttachShader (program, vs);
	glAttachShader (program, fs);

	glLinkProgram (program);
	glUseProgram (program);

	color = glGetUniformLocation (program, "color");

	glUniform4f (color, r, g, b, 1.0);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width, height, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);

	glBegin (GL_QUADS);
	glVertex2f (0, 0);
	glVertex2f (width, 0);
	glVertex2f (width, height);
	glVertex2f (0, height);
	glEnd ();

	glUseProgram (0);
}

static void
draw (Display *dpy, Window window, int width, int height)
{
	GLenum glew_err;

        int visual_attr[] = {
                GLX_RGBA,
                GLX_RED_SIZE,		8,
                GLX_GREEN_SIZE, 	8,
                GLX_BLUE_SIZE,		8,
                GLX_ALPHA_SIZE, 	8,
                GLX_DOUBLEBUFFER,
                GLX_DEPTH_SIZE,		24,
                GLX_STENCIL_SIZE,	8,
                GLX_X_VISUAL_TYPE,	GLX_DIRECT_COLOR,
                None
        };

	/* Window and context setup. */
        XVisualInfo *visual_info = glXChooseVisual(dpy, 0, visual_attr);
        GLXContext ctx = glXCreateContext(dpy, visual_info, NULL, True);
        glXMakeCurrent(dpy, window, ctx);

	glew_err = glewInit();
	if (glew_err != GLEW_OK)
	{
		fprintf (stderr, "glewInit failed: %s\n",
			 glewGetErrorString(glew_err));
		exit (1);
	}

        glViewport(0, 0, width, height);

	/* Frame 1: Draw a solid (magenta) frame using glClear. */
	paint_rgb_using_clear (1, 0, 1);
        glXSwapBuffers (dpy, window);

	/* Frame 2: Draw a solid (yellow) frame using GLSL. */
	paint_rgb_using_glsl (1, 1, 0);
        glXSwapBuffers (dpy, window);

	/* Cleanup */
        glXDestroyContext (dpy, ctx);
}

static void
handle_events(Display *dpy, Window window, int width, int height)
{
        XEvent xev;
        KeyCode quit_code = XKeysymToKeycode (dpy, XStringToKeysym("Q"));

        XNextEvent (dpy, &xev);

        while (1) {
                XNextEvent (dpy, &xev);
                switch (xev.type) {
                case KeyPress:
                        if (xev.xkey.keycode == quit_code) {
                                return;
                        }
                        break;
                case ConfigureNotify:
                        width = xev.xconfigure.width;
                        height = xev.xconfigure.height;
                        break;
                case Expose:
                        if (xev.xexpose.count == 0) {
                                draw (dpy, window, width, height);
                                return;
                        }
                        break;
                }
        }
}

int
main (void)
{
        Display *dpy;
        Window window;

        dpy = XOpenDisplay (NULL);

        if (dpy == NULL) {
                fprintf(stderr, "Failed to open display %s\n",
                        XDisplayName(NULL));
                return 1;
        }

        window = XCreateSimpleWindow(dpy, DefaultRootWindow (dpy),
                                     0, 0, width, height, 0,
                                     BlackPixel (dpy, DefaultScreen (dpy)),
                                     BlackPixel (dpy, DefaultScreen (dpy)));

        XSelectInput(dpy, window,
                     KeyPressMask | StructureNotifyMask | ExposureMask);

        XMapWindow (dpy, window);

        handle_events (dpy, window, width, height);

        XDestroyWindow (dpy, window);
        XCloseDisplay (dpy);

        return 0;
}
