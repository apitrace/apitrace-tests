# http://www.glfw.org/docs/latest/build.html#build_link_cmake_source
set (GLFW_BUILD_EXAMPLES 0)
set (GLFW_BUILD_TESTS 0)
set (GLFW_BUILD_DOCS 0)
set (GLFW_INSTALL 0)
set (GLFW_SUFFIX _gl)
set (GLFW_USE_EGL 0)
set (GLFW_CLIENT_LIBRARY "opengl")
add_subdirectory (${CMAKE_SOURCE_DIR}/thirdparty/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)
set (GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/glfw/include)
set (GLFW_LIBRARY glfw${GLFW_SUFFIX} ${GLFW_LIBRARIES})


add_definitions (-DGLFW_INCLUDE_GL)

include_directories (
    ${OPENGL_INCLUDE_PATH}
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
)

link_libraries (
    ${GLFW_LIBRARY}
    ${GLEW_glew_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${OPENGL_gl_LIBRARY}
)

set (api gl)

add_app_test (
    NAME "gl_default"
    TARGET ${api}_tri
    REF default.ref.txt
)

set (targets
    debug
    tri
    tri_glsl
    gremedy
    varray
    map_buffer
    exception
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (targets ${targets} fork)
endif ()

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.c)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${target}.ref.txt)
        add_app_test(
            NAME ${api}_${target}
            TARGET ${api}_${target}
            REF ${target}.ref.txt
        )
    endif ()
endforeach (target)

add_app_test (
    NAME "gl_varray_drawarrays"
    TARGET ${api}_varray
    REF varray_drawarrays.ref.txt
    ARGS pointer drawarrays
)

add_app_test (
    NAME "gl_varray_elements"
    TARGET ${api}_varray
    REF varray_drawelements.ref.txt
    ARGS pointer drawelements
)

add_app_test (
    NAME "gl_varray_interleaved"
    TARGET ${api}_varray
    REF varray_interleaved.ref.txt
    ARGS interleaved drawarrays
)

add_app_test (
    NAME "gl_map_buffer_arb"
    TARGET ${api}_map_buffer
    REF map_buffer_arb.ref.txt
    ARGS map_buffer_arb
)

add_app_test (
    NAME "gl_map_buffer_1_5"
    TARGET ${api}_map_buffer
    REF map_buffer_1_5.ref.txt
    ARGS map_buffer_1_5
)

add_app_test (
    NAME "gl_map_buffer_range"
    TARGET ${api}_map_buffer
    REF map_buffer_range.ref.txt
    ARGS map_buffer_range
)

add_app_test (
    NAME "gl_debug_khr"
    TARGET ${api}_debug
    REF debug_khr.ref.txt
    ARGS GL_KHR_debug
)

add_app_test (
    NAME "gl_debug_arb"
    TARGET ${api}_debug
    REF debug_arb.ref.txt
    ARGS GL_ARB_debug_output
)

add_app_test (
    NAME "gl_debug_amd"
    TARGET ${api}_debug
    REF debug_amd.ref.txt
    ARGS GL_AMD_debug_output
)

add_app_test (
    NAME "gl_debug_ext"
    TARGET ${api}_debug
    REF debug_ext.ref.txt
    ARGS GL_EXT_debug_marker
)
