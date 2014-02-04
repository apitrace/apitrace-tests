include_directories (
    ${OPENGL_INCLUDE_PATH}
    ${GLUT_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
)

link_libraries (
    ${GLUT_glut_LIBRARY}
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

add_app_test (
    NAME "gl_default_sb"
    TARGET ${api}_tri
    REF default_sb.ref.txt
    ARGS -sb
)

add_app_test (
    NAME "gl_default_db"
    TARGET ${api}_tri
    REF default_db.ref.txt
    ARGS -db
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
