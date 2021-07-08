/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/*
 * \file coherent_memory_write_threaded.cpp
 *
 * Test verifies the correctness of tracing when simultaneously writing
 * into coherent memory from multiple threads.
 *
 * \author Danylo Piliaiev <danylo.piliaiev@globallogic.com>
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <thread>
#include <atomic>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const uint32_t values_count = 256 * 1024;
static const uint32_t fill_value = 0b01010101010101010101010101010101;
static const uint32_t max_thread_count = 4;

static GLuint cs_prog;
static GLuint cs_prog_dummy;

static const GLchar *compute_shader_dummy_text = "#version 430 core\n"
      "layout(local_size_x = 1) in;\n"
      "void main() {}";

static const GLchar *compute_shader_text = "#version 430 core\n"
      "#extension GL_ARB_uniform_buffer_object : require\n"
      "layout(local_size_x = 1, local_size_y = 1) in; \n"
      "\n"
      "layout(location = 0) uniform uint values_count; \n"
      "\n"
      "layout(std430, binding=0) buffer bo { uint data[]; }; \n"
      "\n"
      "void main()\n"
      "{\n"
      "   const uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x;\n"
      "   if (id < values_count)\n"
      "     data[id] *= 2;\n"
      "}\n";

void check_error(void)
{
    GLenum error = glGetError();
    switch (error)
    {
        case GL_NO_ERROR:
            break;
        case GL_OUT_OF_MEMORY:
            exit(EXIT_SKIP);
        default:
            exit(EXIT_FAILURE);
    }
}

static void require_extension(const char *extension, int gl_version = GLAD_GL_VERSION_4_4)
{
    if (!gl_version && !glfwExtensionSupported("extension"))
    {
        fprintf(stderr, "error: %s not supported\n", extension);
        glfwTerminate();
        exit(EXIT_SKIP);
    }
}

static GLuint build_program(const GLchar *shader)
{
    GLuint prog = glCreateProgram();
    int computeShader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(computeShader, 1, &shader, NULL);
    glCompileShader(computeShader);

    int success;
    char infoLog[512];
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        printf("error: shader compilation failed %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glAttachShader(prog, computeShader);
    glLinkProgram(prog);

    glGetProgramiv(prog, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        printf("error: program link failed %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    assert(prog);

    return prog;
}

static GLuint init_ssbo(void)
{
    GLuint data;

    glGenBuffers(1, &data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);

    glBufferStorage(GL_SHADER_STORAGE_BUFFER, values_count * sizeof(uint32_t), nullptr,
        GL_MAP_READ_BIT |
        GL_MAP_WRITE_BIT |
        GL_MAP_PERSISTENT_BIT |
        GL_MAP_COHERENT_BIT);

    uint32_t *coherent_memory = reinterpret_cast<uint32_t*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
        values_count * sizeof(uint32_t),
        GL_MAP_READ_BIT |
        GL_MAP_WRITE_BIT |
        GL_MAP_PERSISTENT_BIT |
        GL_MAP_COHERENT_BIT));

    memset(coherent_memory, fill_value, values_count * sizeof(uint32_t));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    check_error();

    return data;
}

void fence()
{
    GLsync fence;
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
}

uint32_t *map_ssbo(const GLuint ssbo, const uint32_t byte_offset)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    void *coherent_memory = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, byte_offset,
        values_count * sizeof(uint32_t) - byte_offset,
        GL_MAP_READ_BIT |
        GL_MAP_WRITE_BIT |
        GL_MAP_PERSISTENT_BIT |
        GL_MAP_COHERENT_BIT);

    return reinterpret_cast<uint32_t*>(coherent_memory);
}

bool run_test(uint32_t thread_count)
{
    GLuint ssbo_id = init_ssbo();
    uint32_t *data = map_ssbo(ssbo_id, 0);

    glUseProgram(cs_prog_dummy);

    /**
     * Launch several background threads which populate writable coherent buffer,
     * meanwhile the main thread calls glDispatchCompute until background threads are
     * finished which makes apitrace commit writes while coherent buffer is changed from
     * the background threads.
     */

    std::atomic<uint32_t> threads_finished(0);
    std::vector<std::unique_ptr<std::thread>> threads;
    for (uint32_t thread_id = 0; thread_id < thread_count; thread_id++) {
        auto thread = new std::thread([thread_id, thread_count, data, &threads_finished]() {
            for (uint32_t i = thread_id; i < values_count; i += thread_count) {
                data[i] = i;
            }

            threads_finished++;
        });
        threads.emplace_back(std::unique_ptr<std::thread>(thread));
    }

    while (threads_finished.load() < thread_count) {
        glDispatchCompute(1,1,1);
    }

    for (auto& it : threads) {
        it->join();
    }

    glUseProgram(cs_prog);

    glUniform1ui(glGetUniformLocation(cs_prog, "values_count"), values_count);

    const int dim = static_cast<int>(ceil(sqrt(values_count)));
    glDispatchCompute(dim, dim, 1);

    fence();

    for (uint32_t i = 0; i < values_count; i++) {
        if (data[i] != i * 2) {
            printf("Failure on data[%d], expected %d, got %d, thread count %d\n", i, i * 2, data[i], thread_count);
            return false;
        }
    }

    return true;
}
int main(int argc, char **argv)
{
    bool pass = true;

    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(250, 250, argv[0], NULL, NULL);
    if (!window)
    {
        return EXIT_SKIP;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return EXIT_FAILURE;
    }

    require_extension("GL_ARB_uniform_buffer_object");
    require_extension("GL_ARB_buffer_storage");
    require_extension("GL_ARB_map_buffer_range");

    cs_prog_dummy = build_program(compute_shader_dummy_text);
    cs_prog = build_program(compute_shader_text);

    for (uint32_t i = 1; i < max_thread_count; i++)
    {
        pass = run_test(i) && pass;
        if (!pass)
        {
            break;
        }
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return pass ? 0 : EXIT_FAILURE;
}