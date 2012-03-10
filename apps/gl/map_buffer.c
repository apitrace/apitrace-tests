/**************************************************************************
 *
 * Copyright 2012 Jose Fonseca
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

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


static void
parseArgs(int argc, char** argv)
{
}

static void
init(void)
{
	GLenum target = GL_ARRAY_BUFFER;
    GLuint buffers[2];
    GLvoid *ptr;

	if (!GLEW_VERSION_1_5 ||
        !GLEW_ARB_map_buffer_range) {
        exit(0);
	}

	glGenBuffers(2, buffers);
	
    glBindBuffer(target, buffers[0]);
	glBufferData(target, 1000, NULL, GL_STATIC_DRAW);

	ptr = glMapBufferRange(target, 100, 200, GL_MAP_WRITE_BIT);
	memset(ptr, 0, 200);
	glUnmapBuffer(target);

    glBindBuffer(target, buffers[1]);
	glBufferData(target, 2000, NULL, GL_STATIC_DRAW);
	ptr = glMapBufferRange(target, 200, 300, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	memset(ptr, 0, 300);

    glBindBuffer(target, buffers[0]);
	ptr = glMapBufferRange(target, 100, 200, GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	memset(ptr, 0, 200);

    glBindBuffer(target, buffers[1]);
    glFlushMappedBufferRange(target, 20, 30);
    glFlushMappedBufferRange(target, 40, 50);
	glUnmapBuffer(target);

    glBindBuffer(target, buffers[0]);
    glFlushMappedBufferRange(target, 10, 20);
    glFlushMappedBufferRange(target, 30, 40);
	glUnmapBuffer(target);
	
    glMapBufferRange(target, 100, 200, GL_MAP_READ_BIT);
	glUnmapBuffer(target);
}

int main(int argc, char** argv)
{
    parseArgs(argc, argv);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutCreateWindow(argv[0]);
    glewInit();
    init();
    return 0;
}
