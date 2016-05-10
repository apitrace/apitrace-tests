# http://www.glfw.org/docs/latest/build.html#build_link_cmake_source
set (GLFW_BUILD_EXAMPLES 0)
set (GLFW_BUILD_TESTS 0)
set (GLFW_BUILD_DOCS 0)
set (GLFW_INSTALL 0)
set (GLFW_SUFFIX _gl)
set (GLFW_USE_EGL 0)
set (GLFW_USE_RETINA 0)
set (GLFW_CLIENT_LIBRARY "opengl")
add_subdirectory (${CMAKE_SOURCE_DIR}/thirdparty/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)
set (GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include)
set (GLFW_LIBRARY glfw${GLFW_SUFFIX} ${GLFW_LIBRARIES})


add_definitions (-DGLFW_INCLUDE_GL)

include_directories (
    ${OPENGL_INCLUDE_PATH}
    ${GLFW_INCLUDE_DIR}
)

link_libraries (
    glad
    ${GLFW_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${OPENGL_gl_LIBRARY}
)

set (api gl)

set (targets
    child
    config
    compiled_vertex_array
    debug
    default
    dlopen
    tri
    tri_glsl
    tri_glsl_core
    tri_glsl_es2
    gremedy
    varray
    map_buffer
    map_coherent
    program_binary
    exception
    window_resize
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (targets ${targets} fork)
endif ()

if (WIN32)
    set (targets ${targets} wglfont)
endif ()

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.cpp)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})
endforeach (target)

target_link_libraries (${api}_dlopen ${CMAKE_DL_LIBS})

add_app_tests ()
