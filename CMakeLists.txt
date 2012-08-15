cmake_minimum_required (VERSION 2.8)


# Use clang on MacOSX. gcc doesn't support __thread key, and Apple has
# abandoned it for clang.  This must be done before the project is defined.
if (APPLE)
    set (CMAKE_C_COMPILER "clang")
    set (CMAKE_CXX_COMPILER "clang++")
endif ()


project (apitrace-tests)

find_path (APITRACE_SOURCE_DIR NAMES cmake/FindDirectX.cmake PATHS .. ../.. DOC "apitrace source tree" NO_DEFAULT_PATH)
if (NOT EXISTS ${APITRACE_SOURCE_DIR})
    message (WARNING "Please specify path to apitrace source tree via APITRACE_SOURCE_DIR")
endif ()

find_program (APITRACE_EXECUTABLE NAMES apitrace PATHS ${APITRACE_SOURCE_DIR} DOC "apitrace executable")
if (NOT EXISTS ${APITRACE_EXECUTABLE})
    message (WARNING "Please specify path to apitrace executable via APITRACE_EXECUTABLE")
endif ()

include (FindPkgConfig)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set default built type
if (NOT CMAKE_BUILD_TYPE)
   set (CMAKE_BUILD_TYPE Debug
       CACHE
       STRING "Choose the build type, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel."
       FORCE)
endif (NOT CMAKE_BUILD_TYPE)

include (CMakeParseArguments)

find_package (OpenGL)
find_package (GLUT)
find_package (GLEW)

if (WIN32)
    find_package (DirectX)
elseif (PKG_CONFIG_FOUND)
	pkg_check_modules (EGL egl)
	pkg_check_modules (GLESV1 glesv1_cm)
	pkg_check_modules (GLESV2 glesv2)
endif ()

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

enable_testing()

add_subdirectory (apps)
add_subdirectory (traces)

# FIXME: The tests in the cli directory are intended to be high-level
# tests of the apitrace command-line interface which would ideally be
# portable across all platforms. However, these tests all rely on
# doing image comparisons and the current implementation of the
# "apitrace dump-images" command relies on direct invocation of the
# glretrace command.
#
# Someday, we should have more unified commands for replaying traces,
# dumping images, etc. At that point these cli tests should be usable
# with all targets so that we can drop the "if (OPENGL_FOUND)"
# condition here.
if (OPENGL_FOUND)
    add_subdirectory (cli)
    add_subdirectory (trim_stress)
endif ()
