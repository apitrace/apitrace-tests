include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../eglut
    ${EGL_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_PATH}
)

set (api egl)

set (targets
    tri
)

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.c)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})
    target_link_libraries (${api}_${target} eglut ${OPENGL_gl_LIBRARY})
    
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${target}.ref.txt)
        add_app_test(
            NAME ${api}_${target}
            TARGET ${api}_${target}
            REF ${target}.ref.txt
        )
    endif ()
endforeach (target)
