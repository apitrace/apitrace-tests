cmake_minimum_required (VERSION 2.8)


# Use clang on MacOSX. gcc doesn't support __thread key, and Apple has
# abandoned it for clang.  This must be done before the project is defined.
if (APPLE)
    set (CMAKE_C_COMPILER "clang")
    set (CMAKE_CXX_COMPILER "clang++")
endif ()


option (TEST_UNRELIABLE "Run unreliable tests" OFF)


project (apitrace-tests)

find_path (APITRACE_SOURCE_DIR NAMES cmake/FindDirectX.cmake PATHS .. ../.. DOC "apitrace source tree" NO_DEFAULT_PATH)
if (NOT EXISTS ${APITRACE_SOURCE_DIR})
    message (WARNING "Please specify path to apitrace source tree via APITRACE_SOURCE_DIR")
endif ()

find_program (APITRACE_EXECUTABLE NAMES apitrace PATHS ${APITRACE_SOURCE_DIR} DOC "apitrace executable")
if (NOT EXISTS ${APITRACE_EXECUTABLE})
    message (WARNING "Please specify path to apitrace executable via APITRACE_EXECUTABLE")
endif ()

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package (PythonInterp 2.6 REQUIRED)
find_package (PkgConfig)

# Set default built type
if (NOT CMAKE_BUILD_TYPE)
   set (CMAKE_BUILD_TYPE Debug
       CACHE
       STRING "Choose the build type, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel."
       FORCE)
endif (NOT CMAKE_BUILD_TYPE)

include (CMakeParseArguments)

find_package (OpenGL)
find_package (GLEW)

if (WIN32)
    find_package (DirectX)
elseif (PKG_CONFIG_FOUND)
	pkg_check_modules (EGL egl)
	pkg_check_modules (GLESV1 glesv1_cm)
	pkg_check_modules (GLESV2 glesv2)
endif ()

# Check for the presence of several python packages, which are needed to build
# generated tests.
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import PIL"
    OUTPUT_QUIET
    ERROR_QUIET
    RESULT_VARIABLE IMPORT_PIL_RESULT)
if (IMPORT_PIL_RESULT EQUAL 0)
    set (PIL_FOUND 1)
else ()
    message (STATUS "python PIL module not found")
endif ()

include (CheckCXXCompilerFlag)

if (UNIX)
	link_libraries(m)
endif (UNIX)

if (WIN32)
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
	add_definitions (-wd4305) # truncation from 'type1' to 'type2'
endif ()

if (MINGW)
    # Avoid depending on MinGW runtime DLLs
    check_cxx_compiler_flag (-static-libgcc HAVE_STATIC_LIBGCC_FLAG)
    if (HAVE_STATIC_LIBGCC_FLAG)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
        set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc")
    endif ()
    check_cxx_compiler_flag (-static-libstdc++ HAVE_STATIC_LIBSTDCXX_FLAG)
    if (HAVE_STATIC_LIBSTDCXX_FLAG)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++")
        set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libstdc++")
    endif ()
endif ()

if (APPLE)
    # Silence deprecated function warnings
    add_definitions (-Wno-deprecated-declarations)
endif ()

enable_testing()

add_definitions (-DEXIT_SKIP=125)

add_subdirectory (apps)
add_subdirectory (traces)

# FIXME: Some of these tests are not reliable on some platforms:
#
# - They use 64x64 drawables, but on Windows these get bumped to bigger sizes
#   to make space for title bar, buttons, etc.
# - The 64x64 resize is not effctive on some platforms (e.g., xvfb-run)
#
if (OPENGL_FOUND AND PIL_FOUND AND TEST_UNRELIABLE)
    add_subdirectory (cli)
    add_subdirectory (trim_stress)
endif ()
