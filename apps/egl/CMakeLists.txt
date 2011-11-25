add_subdirectory(eglut)

if (OPENGL_FOUND)
	add_subdirectory(gl)
endif ()

if (GLESV1_FOUND)
	add_subdirectory(gles1)
endif ()

if (GLESV2_FOUND)
	add_subdirectory(gles2)
endif ()
