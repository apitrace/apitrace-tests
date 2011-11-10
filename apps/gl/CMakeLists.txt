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

function (ADD_APP_TEST)
    cmake_parse_arguments(
        TEST
        # Options
        ""
        # One value args
        "NAME;TARGET;REF"
        # Multi value args
        "ARGS"
        ${ARGN}
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/driver.py
            --build ${APITRACE_BINARY_DIR}
            --api ${api}
            --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_REF}
            "$<TARGET_FILE:${TEST_TARGET}>"
            ${TEST_ARGS}
    )
endfunction ()

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.c)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${target}.ref.txt)
        add_test(
            NAME ${api}_${target}
            COMMAND python ${CMAKE_SOURCE_DIR}/driver.py
                --build ${APITRACE_BINARY_DIR}
                --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${target}.ref.txt
                "$<TARGET_FILE:${api}_${target}>"
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
