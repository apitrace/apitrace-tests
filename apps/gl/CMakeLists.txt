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

set (targets
    child
    debug
    default
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
endforeach (target)


# Automatically discover tests from *.ref.txt files
file (GLOB TEST_REFS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.ref.txt)
list (SORT TEST_REFS)
foreach (TEST_REF ${TEST_REFS})
    if (APITRACE_EXECUTABLE AND APITRACE_SOURCE_DIR)
        # Search for `//!test` header
        file (READ ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_REF} TEST_REF_DATA 4096)
        string (REGEX MATCH "^//!([_a-zA-Z0-9]+)([^\r\n]*)?" TEST_REF_HEAD "${TEST_REF_DATA}")
        set (TEST_TARGET "${CMAKE_MATCH_1}")
        set (TEST_ARGS ${CMAKE_MATCH_2})

        # Split the remaning arguments
        string (REPLACE " " ";" TEST_ARGS_SPLIT "${TEST_ARGS}")

        if (TEST_TARGET)
            get_filename_component (TEST_NAME ${TEST_REF} NAME_WE)
            add_app_test (
                NAME ${api}_${TEST_NAME}
                TARGET ${api}_${TEST_TARGET}
                REF ${TEST_REF}
                ARGS ${TEST_ARGS_SPLIT}
            )
        endif ()
    endif ()
endforeach ()
