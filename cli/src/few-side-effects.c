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

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

static void
set_2d_projection (void)
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width, height, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
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

	set_2d_projection ();

	/* Frame 1. Perform a few operations, most without side effects. */
	{
		GLint line_width UNUSED, logic_op UNUSED;
		GLboolean blend_enabled UNUSED;

		glGetIntegerv(GL_LINE_WIDTH, &line_width);
		glGetIntegerv(GL_LOGIC_OP_MODE, &logic_op);

		blend_enabled = glIsEnabled(GL_BLEND);

		glEnable(GL_BLEND);

		blend_enabled = glIsEnabled(GL_BLEND);

		glDisable(GL_BLEND);
	}
	
	glXSwapBuffers (dpy, window);
	/* Frame 2. Again, more operations, most with side effects. */
	{
		GLint line_width UNUSED, logic_op UNUSED;
		GLboolean blend_enabled UNUSED;

		glGetIntegerv(GL_LINE_WIDTH, &line_width);
		glGetIntegerv(GL_LOGIC_OP_MODE, &logic_op);

		blend_enabled = glIsEnabled(GL_BLEND);

		glEnable(GL_BLEND);

		blend_enabled = glIsEnabled(GL_BLEND);

		glDisable(GL_BLEND);
	}

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
