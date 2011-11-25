include_directories (
	${EGL_INCLUDE_DIRS}
)

if (X11_FOUND)
	add_library (eglut eglut.c eglut_x11.c)
	target_link_libraries (eglut ${EGL_LIBRARIES} ${X11_X11_LIB})
else ()
	add_library (eglut eglut.c eglut_screen.c)
	target_link_libraries (eglut ${EGL_LIBRARIES})
endif ()

