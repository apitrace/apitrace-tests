include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../eglut
    ${EGL_INCLUDE_DIRS}
    ${GLESV2_INCLUDE_DIRS}
)

set (api gles2)

set (targets
    tri
)

if (X11_FOUND)
    foreach (target ${targets})
	add_executable (${api}_${target} ${target}.c)
	set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})
	target_link_libraries (${api}_${target} ${EGL_LIBRARIES} ${GLESV2_LIBRARIES})
    endforeach (target)
endif ()
