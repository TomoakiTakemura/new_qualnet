# Look for the header files
find_path(TIREM_INCLUDE_DIR NAMES tirem3_dll.h
          PATHS ${CMAKE_SOURCE_DIR}/addons/tirem)

# Look for the library
if (UNIX)
    set(TIREM_ARCH rhel5)
elseif (WIN32)
    set(TIREM_ARCH mingw64)
endif ()
math(EXPR TIREM_BITS "${CMAKE_SIZEOF_VOID_P} * 8")

set(TIREM_LIB_SEARCH_PATH "${TIREM_ARCH}/${TIREM_BITS}")

find_library(TIREM_LIBRARY NAMES tirem340 libtirem340
             PATHS ${CMAKE_SOURCE_DIR}/addons/tirem
             PATH_SUFFIXES ${TIREM_LIB_SEARCH_PATH})

mark_as_advanced(TIREM_INCLUDE_DIR TIREM_LIBRARY)

# Handle the find_package args and set TIREM_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TIREM
    REQUIRED_VARS TIREM_INCLUDE_DIR TIREM_LIBRARY)
