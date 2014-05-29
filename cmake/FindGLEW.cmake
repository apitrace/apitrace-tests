find_path (GLEW_INCLUDE_DIR GL/glew.h)
find_library (GLEW_glew_LIBRARY GLEW)

if (GLEW_INCLUDE_DIR AND GLEW_glew_LIBRARY)
    set (GLEW_FOUND 1)
    find_package_message (GLEW "Found GLEW: ${GLEW_glew_LIBRARY}" "[${GLEW_glew_LIBRARY}][${GLEW_INCLUDE_DIR}]")
endif ()

mark_as_advanced (GLEW_FOUND)
