# Look for the header file.
find_path(PTH_WIN32_INCLUDE_DIR NAMES pthread.h)

# Look for the library (using different cache variables for the static /MT
# and the dynamic /MD version in case USE_FLEXNET changes values between
# CMake invocations).
if (PTH_WIN32_USE_MT)
    find_library(PTH_WIN32_LIBRARY_MT pthreadVC2MT)
    set(PTH_WIN32_LIBRARY ${PTH_WIN32_LIBRARY_MT})
else ()
    find_library(PTH_WIN32_LIBRARY_MD pthreadVC2)
    set(PTH_WIN32_LIBRARY ${PTH_WIN32_LIBRARY_MD})
endif ()

# Find version string.
if (PTH_WIN32_INCLUDE_DIR AND EXISTS "${PTH_WIN32_INCLUDE_DIR}/pthread.h")
    file(STRINGS "${PTH_WIN32_INCLUDE_DIR}/pthread.h" pth_win32_version_str
         REGEX "^#[\t ]*define[\t ]+PTW32_VERSION[\t ]+")
    string(REGEX REPLACE "^#[\t ]*define[\t ]+PTW32_VERSION[\t ]+([0-9,]*)"
           "\\1" PTH_WIN32_VERSION "${pth_win32_version_str}")
    string(REPLACE "," "." PTH_WIN32_VERSION "${PTH_WIN32_VERSION}")
endif ()

# handle the find_package args and set PthreadsWin32_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PthreadsWin32
    REQUIRED_VARS PTH_WIN32_LIBRARY PTH_WIN32_INCLUDE_DIR
    VERSION_VAR PTH_WIN32_VERSION)
