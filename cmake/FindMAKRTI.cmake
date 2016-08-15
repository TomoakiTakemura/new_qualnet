# Look for the header files
find_path(MAKRTI_INCLUDE_DIR MAK/rtiutil/rtiVersion.h
          HINTS RTI_HOME ENV RTI_HOME
          PATH_SUFFIXES include)
find_path(MAKRTI_HLA13_INCLUDE_DIR RTI.hh
          HINTS ${MAKRTI_INCLUDE_DIR}/HLA13)
find_path(MAKRTI_HLA1516_INCLUDE_DIR RTI/RTI1516.h
          HINTS ${MAKRTI_INCLUDE_DIR}/HLA1516)
mark_as_advanced(MAKRTI_INCLUDE_DIR MAKRTI_HLA13_INCLUDE_DIR
    MAKRTI_HLA1516_INCLUDE_DIR)

# Find version string
if (MAKRTI_INCLUDE_DIR AND EXISTS "${MAKRTI_INCLUDE_DIR}/MAK/rtiutil/rtiVersion.h")
    file(STRINGS "${MAKRTI_INCLUDE_DIR}/MAK/rtiutil/rtiVersion.h" makrti_version_str
         REGEX "^#define[\t ]+DtRtiVersionNumber[\t ]+\".*\"")
    string(REGEX REPLACE "^#define[\t ]+DtRtiVersionNumber[\t ]+\"(.*)\".*"
           "\\1" MAKRTI_VERSION "${makrti_version_str}")
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(MAKRTI_lib_suffix "_64")
else ()
    set(MAKRTI_lib_suffix "")
endif ()

# Convenience macro to look for debugging and release versions of a library
# on Windows
macro (find_makrti_lib name)
    string(TOUPPER "${name}" uppername)
    string(REPLACE "-" "_" uppername "${uppername}")
    if (WIN32)
        find_library(MAKRTI_${uppername}_LIBRARY_REL
                     NAMES ${name}${MAKRTI_lib_suffix} lib${name}${MAKRTI_lib_suffix}
                     HINTS RTI_HOME ENV RTI_HOME
                     PATH_SUFFIXES lib)
        find_library(MAKRTI_${uppername}_LIBRARY_DBG
                     NAMES ${name}d${MAKRTI_lib_suffix} lib${name}d${MAKRTI_lib_suffix}
                     HINTS RTI_HOME ENV RTI_HOME
                     PATH_SUFFIXES ${vrlink_lib_dir})
        if (MAKRTI_${uppername}_LIBRARY_REL AND MAKRTI_${uppername}_LIBRARY_DBG)
            set(MAKRTI_${uppername}_LIBRARY
                optimized ${MAKRTI_${uppername}_LIBRARY_REL}
                debug ${MAKRTI_${uppername}_LIBRARY_DBG})
        elseif (MAKRTI_${uppername}_LIBRARY_REL)
            set(MAKRTI_${uppername}_LIBRARY ${MAKRTI_${uppername}_LIBRARY_REL})
        elseif (MAKRTI_${uppername}_LIBRARY_DBG)
            set(MAKRTI_${uppername}_LIBRARY ${MAKRTI_${uppername}_LIBRARY_DBG})
        else ()
            set(MAKRTI_${uppername}_LIBRARY NOTFOUND)
        endif ()
        mark_as_advanced(MAKRTI_${uppername}_LIBRARY
                         MAKRTI_${uppername}_LIBRARY_REL
                         MAKRTI_${uppername}_LIBRARY_DBG)
    else ()
        find_library(MAKRTI_${uppername}_LIBRARY NAMES ${name}${MAKRTI_lib_suffix}
                     HINTS RTI_HOME ENV RTI_HOME
                     PATH_SUFFIXES lib)
        mark_as_advanced(MAKRTI_${uppername}_LIBRARY)
    endif ()
endmacro ()

# Look for some library files
find_makrti_lib(RTI-NG)
find_makrti_lib(fedtime)
find_makrti_lib(rti1516)
find_makrti_lib(fedtime1516)

# handle the find_package args and set MAKRTI_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MAKRTI
    REQUIRED_VARS MAKRTI_INCLUDE_DIR MAKRTI_HLA13_INCLUDE_DIR
        MAKRTI_HLA1516_INCLUDE_DIR
        MAKRTI_RTI_NG_LIBRARY MAKRTI_FEDTIME_LIBRARY
        MAKRTI_RTI1516_LIBRARY MAKRTI_FEDTIME1516_LIBRARY
    VERSION_VAR MAKRTI_VERSION)
