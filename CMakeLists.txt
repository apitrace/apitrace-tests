cmake_minimum_required (VERSION 2.8)

project (apitrace-tests)

# Set default built type
if (NOT CMAKE_BUILD_TYPE)
   set (CMAKE_BUILD_TYPE Debug
       CACHE
       STRING "Choose the build type, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel."
       FORCE)
endif (NOT CMAKE_BUILD_TYPE)

include (CMakeParseArguments)

find_package (OpenGL REQUIRED)
find_package (GLUT)

find_path (GLEW_INCLUDE_DIR GL/glew.h /usr/include/GL)
find_library (GLEW_glew_LIBRARY GLEW /usr/lib)

if (UNIX)
	link_libraries(m)
endif (UNIX)

if (WIN32)
	# Nobody likes to include windows.h:
	# - Microsoft's GL/gl.h header depends on windows.h but doesn't include it;
	# - GLEW temporarily defines the necessary defines but undefines them later
	# - certain GLUT distributions don't include it;
	# - most of our programs are meant to be portable so don't include it.
	#
	# We could try to replicate the windows.h definitions required by
	# GL/gl.h, but the build time savings don't compensate the constant
	# headaches that brings, so instead we force windows.h to be included
	# on every file.
	if (MSVC)
		add_definitions (-FIwindows.h)
	else (MSVC)
		add_definitions (--include windows.h)
	endif (MSVC)

	# Don't define min/max macros
	add_definitions (-DNOMINMAX)

	# MSVC & MinGW only define & use APIENTRY
	add_definitions (-DGLAPIENTRY=__stdcall)
endif ()

if (MSVC)
	# Enable math constants defines
	add_definitions (-D_USE_MATH_DEFINES)

	# Silence several MSVC pedantic warnings
	add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
	add_definitions (-wd4244) # conversion' conversion from 'type1' to 'type2', possible loss of data
endif ()

if (NOT APITRACE_BINARY_DIR) 
    set (APITRACE_BINARY_DIR "${CMAKE_BINARY_DIR}/.." CACHE PATH "apitrace build directory")
endif ()
if (NOT EXISTS ${APITRACE_BINARY_DIR})
    message (SEND_ERROR "Invalid APITRACE_BINARY_DIR")
endif ()

enable_testing()

add_subdirectory (apps)

