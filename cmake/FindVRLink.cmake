# Look for the header file
find_path(VRLINK_INCLUDE_DIR vlutil/vlVersion.h
          HINTS VLHOME ENV VLHOME
          PATH_SUFFIXES include)
find_path(VRLINK_COMPAT_INCLUDE_DIR vl/exConnInit.h
          HINTS ${VRLINK_INCLUDE_DIR} ${VRLINK_INCLUDE_DIR}/compatibility)
mark_as_advanced(VRLINK_INCLUDE_DIR VRLINK_COMPAT_INCLUDE_DIR)

# Find version string
if (VRLINK_INCLUDE_DIR AND EXISTS "${VRLINK_INCLUDE_DIR}/vlutil/vlVersion.h")
    file(STRINGS "${VRLINK_INCLUDE_DIR}/vlutil/vlVersion.h" vrlink_version_str
         REGEX "^#define[\t ]+VR_LINK_VERSION_STRING[\t ]+\".*\"")
    string(REGEX REPLACE "^#define[\t ]+VR_LINK_VERSION_STRING[\t ]+\"(.*)\".*"
           "\\1" VRLINK_VERSION "${vrlink_version_str}")
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(vrlink_lib_dir lib64)
else ()
    set(vrlink_lib_dir lib)
endif ()

# Convenience macro to look for debugging and release versions of a library
# on Windows
macro (find_vrlink_lib name)
    string(TOUPPER "${name}" uppername)
    if (WIN32)
        find_library(VRLINK_${uppername}_LIBRARY_REL NAMES ${name}
                     HINTS VLHOME ENV VLHOME
                     PATH_SUFFIXES ${vrlink_lib_dir})
        find_library(VRLINK_${uppername}_LIBRARY_DBG NAMES ${name}d
                     HINTS VLHOME ENV VLHOME
                     PATH_SUFFIXES ${vrlink_lib_dir})
        if (VRLINK_${uppername}_LIBRARY_REL AND VRLINK_${uppername}_LIBRARY_DBG)
            set(VRLINK_${uppername}_LIBRARY
                optimized ${VRLINK_${uppername}_LIBRARY_REL}
                debug ${VRLINK_${uppername}_LIBRARY_DBG})
        elseif (VRLINK_${uppername}_LIBRARY_REL)
            set(VRLINK_${uppername}_LIBRARY ${VRLINK_${uppername}_LIBRARY_REL})
        elseif (VRLINK_${uppername}_LIBRARY_DBG)
            set(VRLINK_${uppername}_LIBRARY ${VRLINK_${uppername}_LIBRARY_DBG})
        else ()
            set(VRLINK_${uppername}_LIBRARY NOTFOUND)
        endif ()
        mark_as_advanced(VRLINK_${uppername}_LIBRARY
                         VRLINK_${uppername}_LIBRARY_REL
                         VRLINK_${uppername}_LIBRARY_DBG)
    else ()
        find_library(VRLINK_${uppername}_LIBRARY NAMES ${name}
                     HINTS VLHOME ENV VLHOME
                     PATH_SUFFIXES ${vrlink_lib_dir})
        mark_as_advanced(VRLINK_${uppername}_LIBRARY)
    endif ()
endmacro ()

# Look for some library files
find_vrlink_lib(vlutil)
find_vrlink_lib(vl)
find_vrlink_lib(matrix)
find_vrlink_lib(mtl)
find_vrlink_lib(vlDIS)
find_vrlink_lib(vlHLA13)
find_vrlink_lib(vlHLA1516)

# handle the find_package args and set VRLINK_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRLINK
    REQUIRED_VARS VRLINK_INCLUDE_DIR VRLINK_COMPAT_INCLUDE_DIR
        VRLINK_VLUTIL_LIBRARY VRLINK_VL_LIBRARY VRLINK_MATRIX_LIBRARY
        VRLINK_MTL_LIBRARY VRLINK_VLDIS_LIBRARY VRLINK_VLHLA13_LIBRARY
        VRLINK_VLHLA1516_LIBRARY
    VERSION_VAR VRLINK_VERSION)

if (VRLINK_FOUND)
    set(VRLINK_INCLUDE_DIRS ${VRLINK_INCLUDE_DIR} ${VRLINK_COMPAT_INCLUDE_DIR})
endif ()
  