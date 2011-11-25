include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../eglut
    ${EGL_INCLUDE_DIRS}
    ${GLESV1_INCLUDE_DIRS}
)

set (api gles1)

set (targets
    clear
    tri
)

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.c)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})
    target_link_libraries (${api}_${target} eglut ${GLESV1_LIBRARIES})
endforeach (target)
