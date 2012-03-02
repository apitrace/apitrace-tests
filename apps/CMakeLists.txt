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
        NAME app_${TEST_NAME}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/driver.py
            --apitrace ${APITRACE_EXECUTABLE}
            --api ${api}
            --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_REF}
            --
            "$<TARGET_FILE:${TEST_TARGET}>"
            ${TEST_ARGS}
    )
endfunction ()

if (OPENGL_FOUND)
    add_subdirectory (gl)
endif ()

if (EGL_FOUND)
    add_subdirectory (egl)
endif ()
