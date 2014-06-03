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

    if (APITRACE_EXECUTABLE AND APITRACE_SOURCE_DIR)
        add_test(
            NAME app_${TEST_NAME}
            COMMAND
            ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/app_driver.py
                --apitrace ${APITRACE_EXECUTABLE}
                --apitrace-source ${APITRACE_SOURCE_DIR}
                --api ${api}
                --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_REF}
                --
                "$<TARGET_FILE:${TEST_TARGET}>"
                ${TEST_ARGS}
        )
    endif ()
endfunction ()

if (OPENGL_FOUND AND GLEW_FOUND)
    add_subdirectory (gl)
endif ()

if (EGL_FOUND)
    add_subdirectory (egl)
endif ()

if (DirectX_FXC_EXECUTABLE)
    add_subdirectory (d3dcommon)
endif ()

if (DirectX_D3D11_1_FOUND)
    add_subdirectory (d3d11_1)
endif ()

if (DirectX_D3D11_FOUND)
    add_subdirectory (d3d11)
endif ()

if (DirectX_D3D10_1_FOUND)
    add_subdirectory (d3d10_1)
endif ()

if (DirectX_D3D10_FOUND)
    add_subdirectory (d3d10)
endif ()

if (DirectX_D3D9_FOUND)
    add_subdirectory (d3d9)
endif ()

if (DirectX_D3D8_FOUND)
    add_subdirectory (d3d8)
endif ()

if (DirectX_D3D_FOUND)
    add_subdirectory (d3d7)
endif ()
