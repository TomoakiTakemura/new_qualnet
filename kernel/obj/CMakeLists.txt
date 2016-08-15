set(supported_glibc_gcc
    2.19/4.8
    2.15/4.6
    2.12/4.4)

if (UNIX)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(BUILD_PLATFORM_BASE "linux" CACHE INTERNAL
            "Identifier string for base build platform" FORCE)
    else ()
        set(BUILD_PLATFORM_BASE "linux-x64" CACHE INTERNAL
            "Identifier string for base build platform" FORCE)
    endif ()
elseif (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(BUILD_PLATFORM_BASE "windows" CACHE INTERNAL
            "Identifier string for base build platform" FORCE)
    else ()
        set(BUILD_PLATFORM_BASE "windows-x64" CACHE INTERNAL
            "Identifier string for base build platform" FORCE)
    endif ()
endif ()

set(BUILD_PLATFORM CACHE STRING "Identifier string for build platform")
if (NOT BUILD_PLATFORM)
    # Couldn't find BUILD_PLATFORM cache variable, need to detect it
    if (UNIX)
        execute_process(COMMAND getconf GNU_LIBC_VERSION
                        OUTPUT_VARIABLE glibc_version_str)
        if (glibc_version_str MATCHES "^glibc ([0-9]+\\.[0-9]+)")
            set(glibc_version "${CMAKE_MATCH_1}")
            message(STATUS "Found glibc version: ${glibc_version}")
        else ()
            message(FATAL_ERROR "Could not determine glibc version")
        endif ()
        if (CMAKE_CXX_COMPILER_VERSION MATCHES "^([0-9]+\\.[0-9]+)")
            set(gcc_version "${CMAKE_MATCH_1}")
        else ()
            # workaround for Ubuntu 12.04 LTS's CMake not setting
            # CMAKE_CXX_COMPILER_VERSION properly
            execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                            OUTPUT_VARIABLE gcc_version_str)
            if (gcc_version_str MATCHES "^.*\\(.*\\) ([0-9]+\\.[0-9]+)")
                set(gcc_version "${CMAKE_MATCH_1}")
            else ()
                message(FATAL_ERROR "Could not determine gcc version; you may need to upgrade CMake")
            endif ()
        endif ()
        message(STATUS "Found gcc version: ${gcc_version}")

        if (EXISTS "${CMAKE_SOURCE_DIR}/addons/kernel" AND
            EXISTS "${CMAKE_SOURCE_DIR}/kernel/src")
            # for internal developer builds, just use the exact glibc and gcc versions
            set(found_platform_spec glibc-${glibc_version}-gcc-${gcc_version})
            set(is_exact_match 1)
        else ()
            # sanity check: verify that we have at least one applicable
            # precompiled object for whichever of linux or linux-x64 we're
            # building for
            file(GLOB private_objects "${CMAKE_SOURCE_DIR}/kernel/obj/${BUILD_TARGET}-main-${BUILD_PLATFORM_BASE}-glibc-*-gcc-*${CMAKE_CXX_OUTPUT_EXTENSION}")
            if (NOT private_objects)
                message(FATAL_ERROR "The source installation at ${CMAKE_SOURCE_DIR} does not support building ${BUILD_TARGET} for platform ${BUILD_PLATFORM_BASE}")
            endif ()

            foreach (platform_spec ${supported_glibc_gcc})
                string(REGEX MATCH "^([0-9]+\\.[0-9]+)/([0-9]+\\.[0-9]+)$"
                       dummy "${platform_spec}")
                set(glibc_test_version "${CMAKE_MATCH_1}")
                set(gcc_test_version "${CMAKE_MATCH_2}")
                if ((glibc_version VERSION_LESS glibc_test_version) OR
                    (gcc_version VERSION_LESS gcc_test_version))
                    # too old for this set of precompiled objects
                elseif (EXISTS "${CMAKE_SOURCE_DIR}/kernel/obj/${BUILD_TARGET}-main-${BUILD_PLATFORM_BASE}-glibc-${glibc_test_version}-gcc-${gcc_test_version}${MPI_SUFFIX}${CMAKE_CXX_OUTPUT_EXTENSION}")
                    set(found_platform_spec "glibc-${glibc_test_version}-gcc-${gcc_test_version}")
                    if ((glibc_version VERSION_EQUAL glibc_test_version) AND
                        (gcc_version VERSION_EQUAL gcc_test_version))
                        set(is_exact_match 1)
                    endif ()
                    break ()
                endif ()
            endforeach ()
        endif ()
        if (NOT found_platform_spec)
            message(FATAL_ERROR "The source installation at ${CMAKE_SOURCE_DIR} does not support building with glibc <= ${glibc_version} and gcc <= ${gcc_version}")
        endif ()

        set(BUILD_PLATFORM "${BUILD_PLATFORM_BASE}-${found_platform_spec}"
            CACHE STRING "Identifier string for build platform" FORCE)
        if (is_exact_match)
            message(STATUS "Found build platform: ${BUILD_PLATFORM}")
        else ()
            message(STATUS "Found build platform: ${BUILD_PLATFORM} (best match from supported platforms)")
        endif ()
    elseif (WIN32)
        if (MSVC12)
            set(msvc_version vc12)
        elseif (MSVC10)
            set(msvc_version vc10)
        elseif (MSVC90)
            set(msvc_version vc9)
        else ()
            message(FATAL_ERROR "Only Visual Studio versions 2008, 2010 and 2013 are supported on Windows")
        endif ()
        set(BUILD_PLATFORM "${BUILD_PLATFORM_BASE}-${msvc_version}"
            CACHE STRING "Identifier string for build platform" FORCE)
        message(STATUS "Found build platform: ${BUILD_PLATFORM}")
    endif (UNIX)
endif ()

# Perform some sanity checks to make sure the installation supports
# the detected build platform
set(sanity_checks_passed 1)

if (BUILD_PLATFORM_BASE STREQUAL "linux")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/include/osdeps/linux/i386_atomic.h")
        set(sanity_checks_passed 0)
    endif ()
elseif (BUILD_PLATFORM_BASE STREQUAL "linux-x64")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/include/osdeps/linux/x86-64_atomic.h")
        set(sanity_checks_passed 0)
    endif ()
elseif (BUILD_PLATFORM_BASE STREQUAL "windows" OR
        BUILD_PLATFORM_BASE STREQUAL "windows-x64")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/include/osdeps/windows/i386_atomic.h")
        set(sanity_checks_passed 0)
    endif ()
endif ()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/addons/kernel" AND
    NOT EXISTS "${CMAKE_SOURCE_DIR}/kernel/obj/${BUILD_TARGET}-main-${BUILD_PLATFORM}${MPI_SUFFIX}${CMAKE_CXX_OUTPUT_EXTENSION}")
    set(sanity_checks_passed 0)
endif ()

if (NOT sanity_checks_passed)
    message(FATAL_ERROR "The source installation at ${CMAKE_SOURCE_DIR} does not support building ${BUILD_TARGET} for platform ${BUILD_PLATFORM}")
endif ()

# Generate a cmake script for installing private objects: only for Scalable
# internal builds, to assist in creating the source archive.
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/install_private_objects.cmake.in)
    configure_file(install_private_objects.cmake.in install_private_objects.cmake @ONLY)
    add_custom_target(install-private-objects
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/install_private_objects.cmake)
endif ()
