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

set (subdir gl)

set (targets
    tri
    gremedy
    varray
)

foreach (target ${targets})
    add_executable (${subdir}_${target} ${target}.c)
    set_target_properties (${subdir}_${target} PROPERTIES OUTPUT_NAME ${target})

    add_test(
        NAME ${subdir}_${target}
        COMMAND python ${CMAKE_SOURCE_DIR}/driver.py
            --build ${APITRACE_BINARY_DIR}
            --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${target}.ref.txt
            ${CMAKE_CURRENT_BINARY_DIR}/${target}
    )
endforeach (target)
