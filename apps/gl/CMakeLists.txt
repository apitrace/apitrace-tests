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

set (targets
    tri
    gremedy
    varray
)

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
