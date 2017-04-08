if (NOT WIN32 AND NOT APPLE)
	add_subdirectory (gles1)
	add_subdirectory (gles2)
endif ()

if (NOT WIN32 AND NOT APPLE)
endif ()

if (OPENGL_FOUND)
	add_subdirectory (gl)
endif ()
