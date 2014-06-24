if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package (X11)
endif ()

if (X11_FOUND AND OPENGL_FOUND AND GLEW_FOUND)

    include_directories (
        ${GLEW_INCLUDE_DIR}
        ${OPENGL_INCLUDE_PATH}
        ${X11_INCLUDE_DIR}
    )

    link_libraries (
        ${GLEW_glew_LIBRARY}
        ${OPENGL_gl_LIBRARY}
        ${X11_X11_LIB}
    )

    add_executable (glxsimple glxsimple.c)
    add_executable (few-side-effects few-side-effects.c)

endif ()
