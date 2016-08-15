# Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
#                          600 Corporate Pointe
#                          Suite 1200
#                          Culver City, CA 90230
#                          info@scalable-networks.com
#
# This source code is licensed, not sold, and is subject to a written
# license agreement.  Among other things, no portion of this source
# code may be copied, transmitted, disclosed, displayed, distributed,
# translated, used as the basis for a derivative work, or used, in
# whole or in part, for any program or purpose other than its intended
# use in compliance with the license agreement as part of the QualNet
# software.  This source code and certain of the algorithms contained
# within it are confidential trade secrets of Scalable Network
# Technologies, Inc. and may not be used as the basis for any other
# software, hardware, product or service.

cmake_minimum_required(VERSION 3.1)

set(MSVC_INCREMENTAL_DEFAULT OFF)

set(CMAKE_USER_MAKE_RULES_OVERRIDE
    ${CMAKE_CURRENT_LIST_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
    ${CMAKE_CURRENT_LIST_DIR}/cmake/cxx_flag_overrides.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CMakeDependentOption)
include(FeatureSummary)
include(target)
include(installutils)
include(collection)

project(${BUILD_TARGET})

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_GENERATOR MATCHES "^Visual Studio")
    option(BUILD_VS_FOLDERS "Enable grouping of projects in Visual Studio solution (not supported in Visual C++ 2008 Express Edition)" OFF)
    set_property(GLOBAL
      PROPERTY USE_FOLDERS ${BUILD_USE_FOLDERS})
endif ()

enable_testing()
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME base)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE
      PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel")
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(EXTRA_COMPILE_FLAGS "-UNDEBUG")
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -msse2 -mfpmath=sse")
  endif ()
elseif (MSVC)
  set(EXTRA_COMPILE_FLAGS "-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SECURE_SCL=0 -DWIN32 -D__WIN32__")
endif ()

option(USE_MPI "Compile using MPI" OFF)
if (USE_MPI)
    find_package(MPI REQUIRED)
    if (MPI_CXX_COMPILE_FLAGS)
        set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
    endif ()
    set(MPI_SUFFIX "-mpi")
else ()
    set(MPI_SUFFIX "")
endif ()

add_subdirectory(kernel)
add_subdirectory(include)
add_subdirectory(libraries)
add_subdirectory(interfaces)   # Note: interfaces must come after kernel
                               # in order for the BUILD_PLATFORM variable
                               # detected by kernel/obj to be available for
                               # interfaces/vrlink.
add_subdirectory(addons)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/contributed)
    add_subdirectory(contributed)
endif ()
add_subdirectory(main)
add_subdirectory(gui)
add_subdirectory(license_dir)

find_package(Eigen3 REQUIRED)
add_global_system_include_dir_absolute(${EIGEN3_INCLUDE_DIR})

find_package(Boost 1.57 REQUIRED)
add_global_system_include_dir_absolute(${Boost_INCLUDE_DIRS})
add_global_definitions(-DBOOST_ALL_NO_LIB)

if (UNIX)
    add_global_link_libs(dl pthread)
elseif (WIN32)
    add_global_link_libs(ws2_32)
    if (USE_FLEXNET)
        set(PTH_WIN32_USE_MT 1)
    endif ()
    find_package(PthreadsWin32 REQUIRED)
    if (USE_FLEXNET)
        add_global_definitions(-DPTW32_STATIC_LIB)
    endif ()
    add_global_system_include_dir_absolute(${PTH_WIN32_INCLUDE_DIR})
    add_global_link_libs(${PTH_WIN32_LIBRARY})
endif ()

add_definitions(${ALL_DEFINES})
add_definitions(-DPARALLEL)
if (USE_MPI)
  add_definitions(-DUSE_MPI)
endif ()
if (IS_EXATA)
  add_definitions(-DEXATA -DEXATA_CYBER)
endif ()

include_directories(SYSTEM ${ALL_SYSTEM_INCLUDES})
include_directories(${ALL_INCLUDES})
if (USE_MPI AND MPI_CXX_INCLUDE_PATH)
    include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})
endif ()

if (MSVC)
  if (USE_FLEXNET)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -D_HAS_ITERATOR_DEBUGGING=0")
  endif ()
  # replace /MD with /MT only for this directory -- some subprojects
  # might require /MD instead
  foreach (flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    string(REGEX REPLACE "/D NDEBUG" "" ${flag_var} "${${flag_var}}")
    if (USE_FLEXNET)
      string(REGEX REPLACE "/MDd?" "/MT" ${flag_var} "${${flag_var}}")
      string(REGEX REPLACE "/D_DEBUG" "" ${flag_var} "${${flag_var}}")
    endif ()
  endforeach ()
endif ()

if (EXISTS ${CMAKE_SOURCE_DIR}/addons/kernel)
  include(addons/kernel/main_target.cmake)
else ()
  include(kernel/obj/main_target.cmake)
endif ()

if (EXISTS ${CMAKE_SOURCE_DIR}/addons/kernel)
  include(addons/kernel/private.cmake)
else ()
  include(kernel/obj/import_private.cmake)
endif ()
if (EXISTS ${CMAKE_SOURCE_DIR}/kernel/src)
  include(kernel/src/kernel.cmake)
else ()
  include(kernel/obj/import_kernel.cmake)
endif ()

add_library(simlib_core STATIC ${ALL_CORE_SRCS})
set_target_properties(simlib_core
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
             FOLDER "${BUILD_TARGET}")
target_link_libraries(simlib_core kernel_core)

add_library(simlib STATIC ${ALL_SRCS})
# For debug builds, disable message recycling so that valgrind is useful
target_compile_definitions(simlib PRIVATE
    $<$<CONFIG:Debug>:MESSAGE_NO_RECYCLE>
)
set_target_properties(simlib
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
             FOLDER "${BUILD_TARGET}")
target_link_libraries(simlib simlib_core private)

kernel_link(kernel_core private simlib)
private_link(kernel simlib)

# simlib needs to go first in case e.g. sqlite3 is built as a static library
# in lib/
set(ALL_LINK_LIBS simlib simlib_core ${ALL_LINK_LIBS} CACHE INTERNAL "libraries to link")

add_executable(${BUILD_TARGET} ${main_source})

if (WIN32)
    # sanity check: check .lib.platform tags to see if any mismatch the
    # platform that we're building for
    foreach (libfile ${ALL_LINK_LIBS})
        if (IS_ABSOLUTE "${libfile}" AND
            EXISTS "${libfile}" AND EXISTS "${libfile}.platform")
            file(READ "${libfile}.platform" platform_text)
            string(STRIP "${platform_text}" platform_text)
            if (NOT (platform_text STREQUAL BUILD_PLATFORM))
                message(SEND_ERROR "Library ${libfile} is for platform ${platform_text} instead of expected ${BUILD_PLATFORM}")
            endif ()
        endif ()
    endforeach ()
endif ()

target_link_libraries(${BUILD_TARGET} ${ALL_LINK_LIBS})
if (USE_MPI AND MPI_CXX_LIBRARIES)
    target_link_libraries(${BUILD_TARGET} ${MPI_CXX_LIBRARIES})
endif ()
if (USE_MPI AND MPI_CXX_LINK_FLAGS)
  set_target_properties(${BUILD_TARGET}
    PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
endif ()
set_target_properties(${BUILD_TARGET}
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
             RUNTIME_OUTPUT_DIRECTORY bin
             RUNTIME_OUTPUT_DIRECTORY_RELEASE bin
             RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO bin
             RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL bin
             RUNTIME_OUTPUT_DIRECTORY_DEBUG bin
             FOLDER ${BUILD_TARGET})

if (USE_MPI)
  set(BUILD_TARGET_SIMEXEC_REAL "${BUILD_TARGET_SIMEXEC}_mpi")
else ()
  set(BUILD_TARGET_SIMEXEC_REAL "${BUILD_TARGET_SIMEXEC}")
endif ()
set_target_properties(${BUILD_TARGET}
  PROPERTIES OUTPUT_NAME ${BUILD_TARGET_SIMEXEC_REAL})

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/lmgr)
  # Add License Manager libraries to link line, if needed
  include(lib/lmgr/lmgr.cmake)
endif ()

#license TS preptool
if (USE_FLEXNET)
  set(PREPTOOL_DIR "${CMAKE_SOURCE_DIR}/license_dir/${BUILD_PLATFORM_BASE}")
  set(BIN_PREPTOOL_DIR "${CMAKE_BINARY_DIR}/license_dir/${BUILD_PLATFORM_BASE}")
  set(PREPTOOL "${PREPTOOL_DIR}/preptool${CMAKE_EXECUTABLE_SUFFIX}")
  set(SCALABLE_LIB_SRC "scalable_libFNP${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(SCALABLE_LIB "lib${BUILD_TARGET_SIMEXEC}_scalable_FNP${CMAKE_SHARED_LIBRARY_SUFFIX}")

  set(PREPARCHIVE ${PREPTOOL_DIR}/preparchive_11.12.0.yaa)
  set(PREPTOOL_XML ${BIN_PREPTOOL_DIR}/scalable_main.xml)
  configure_file(${CMAKE_SOURCE_DIR}/license_dir/scalable_main.xml.in ${PREPTOOL_XML} @ONLY)

  if (UNIX)
    # Check for the lsb dynamic loader needed by preptool, and if not found
    # provide an error message other than the very misleading
    # .../preptool: Command not found
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(lsb_ld "/lib64/ld-lsb-x86-64.so.3")
    else ()
      set(lsb_ld "/lib/ld-lsb.so.3")
    endif ()

    if (NOT EXISTS "${lsb_ld}")
      message(FATAL_ERROR "${BUILD_TARGET} requires the LSB version 3 compatibility layer to compile.  For example, on Ubuntu, run:
    sudo apt-get install lsb-core")
    endif ()
  endif ()

  add_custom_command(TARGET ${BUILD_TARGET} POST_BUILD
    COMMAND ${PREPTOOL} -v ${PREPTOOL_XML}
      -p ${PREPARCHIVE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  # Add rpath of $ORIGIN so the binary can find *scalable_libFNP.so
  set_target_properties(${BUILD_TARGET}
    PROPERTIES INSTALL_RPATH "\$ORIGIN")
  # With NDT interface, need link-time RPATH to include location of
  # LoggingAPI library (and more importantly, its binary dependencies on
  # boost).
  if (NOT WITH_NDT_INTERFACE)
    set_target_properties(${BUILD_TARGET}
      PROPERTIES BUILD_WITH_INSTALL_RPATH 1)
  endif ()
  install(FILES ${CMAKE_BINARY_DIR}/bin/${SCALABLE_LIB}
          DESTINATION bin)
endif ()

# Only install simulator executable if it was built with FlexNet; it's better
# to risk a broken installer than to risk an installer with an unlicensed
# simulator.
if (USE_FLEXNET)
    install(TARGETS ${BUILD_TARGET} RUNTIME DESTINATION bin)

    # install non-system libraries which were linked against
    set(sim_search_path)
    foreach (lib_for_search_path ${ALL_LINK_LIBS})
        if (IS_ABSOLUTE "${lib_for_search_path}" AND
            EXISTS "${lib_for_search_path}")
            get_filename_component(search_dir "${lib_for_search_path}" PATH)
            list(APPEND sim_search_path "${search_dir}")
            if (WIN32 AND search_dir MATCHES "/lib")
                string(REPLACE "/lib" "/bin" search_dir "${search_dir}")
                if (EXISTS "${search_dir}")
                    list(APPEND sim_search_path "${search_dir}")
                endif ()
            endif ()
        endif ()
    endforeach ()

    list(REMOVE_DUPLICATES sim_search_path)

    install(CODE "
        message(STATUS \"Calculating required libraries...\")
        include(GetPrerequisites)
        set(search_path \"${sim_search_path}\")
        get_prerequisites(\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${BUILD_TARGET_SIMEXEC_REAL}${CMAKE_EXECUTABLE_SUFFIX}
            sim_prereqs 1 1 \"\" \"\${search_path}\")
        foreach (libfile \${sim_prereqs})
            message(STATUS \"libfile = \${libfile}\")
            if (WIN32 AND (libfile STREQUAL \"WINTRUST.dll\"))
                # If DirectX SDK is installed, this somehow resolves to a
                # 32-bit DLL in that SDK which is then not considered a
                # system path; so skip that one as WINTRUST.dll should be
                # a system library.
            else ()
                gp_resolve_item(\"\" \${libfile} \"\" \"\${search_path}\" libfile_abs)
                file(INSTALL \${libfile_abs} DESTINATION \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)
                if (IS_SYMLINK \${libfile_abs})
                    get_filename_component(libfile_target \${libfile_abs} REALPATH)
                    file(INSTALL \${libfile_target} DESTINATION \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)
                endif ()
            endif ()
        endforeach ()")
endif ()

# Build extra utilities registered in ALL_UTILITY_TARGET_INCLUDES
foreach (includefile ${ALL_UTILITY_TARGET_INCLUDES})
  include(${includefile})
endforeach ()

add_scenario_dir(default demo inter-channel_interference samples)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scenarios/CMakeLists.txt)
    add_subdirectory(scenarios)
elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../scenarios/CMakeLists.txt)
    add_subdirectory(../scenarios scenarios)
endif ()

find_package(Doxygen)
if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    set(DOX_PROJECT_NUMBER ${BUILD_TARGET_VERSION})
    string(REPLACE ";" " \\\n    " DOXYGEN_INPUTS "${ALL_DOXYGEN_INPUTS}")
    string(REPLACE ";" " \\\n    " DOXYGEN_EXCLUDES "${ALL_DOXYGEN_EXCLUDES}")
    configure_file(Doxyfile.in Doxyfile @ONLY)
    configure_file(Doxyfile_core.in Doxyfile_core @ONLY)

    add_custom_target(apidocs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/documentation/apidocs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_target(apidocs-core
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/documentation/apidocs-core
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile_core
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
add_feature_info(apidocs "DOXYGEN_FOUND;AND;DOXYGEN_DOT_FOUND" "Build API documentation with \"make apidocs\"")

install(DIRECTORY data DESTINATION .
        PATTERN ".svn" EXCLUDE)
install(FILES "License.txt" DESTINATION .)
configure_file(cmake/target.cmake.in cmake/install_target.cmake @ONLY)
install_clone(PROGRAMS bin/urban_grid.pl)
install(DIRECTORY documentation DESTINATION .
        COMPONENT documentation
        PATTERN ".svn" EXCLUDE
        PATTERN "checkout-docs" EXCLUDE
        PATTERN "military_radios" EXCLUDE
        PATTERN "README" EXCLUDE
        PATTERN "README-SVN-COMMIT-INSTRUCTIONS" EXCLUDE
        PATTERN "HelpEngineScripts" EXCLUDE
        PATTERN "set-svn-properties.sh" EXCLUDE
        PATTERN "set_svn_properties.sh" EXCLUDE)
if (WITH_MILITARY_ITAR AND
    EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/documentation/military_radios)
    file(GLOB mr_docs ${CMAKE_CURRENT_SOURCE_DIR}/documentation/military_radios/*)
    foreach (mr_doc ${mr_docs})
        if (mr_doc MATCHES "/\\.svn$")
            # skip
        elseif (IS_DIRECTORY ${mr_doc})
            install(DIRECTORY ${mr_doc} DESTINATION documentation
                COMPONENT documentation
                PATTERN ".svn" EXCLUDE
                PATTERN "checkout-docs" EXCLUDE
                PATTERN "military_radios" EXCLUDE
                PATTERN "README" EXCLUDE
                PATTERN "README-SVN-COMMIT-INSTRUCTIONS" EXCLUDE
                PATTERN "HelpEngineScripts" EXCLUDE
                PATTERN "set-svn-properties.sh" EXCLUDE
                PATTERN "set_svn_properties.sh" EXCLUDE)
        else ()
            install(FILES ${mr_doc} DESTINATION documentation
                COMPONENT documentation)
        endif ()
    endforeach ()
endif ()
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../documentation)
    install(DIRECTORY ../documentation DESTINATION .
            COMPONENT documentation
            PATTERN ".svn" EXCLUDE
            PATTERN "checkout-docs" EXCLUDE
            PATTERN "military_radios" EXCLUDE
            PATTERN "README" EXCLUDE
            PATTERN "README-SVN-COMMIT-INSTRUCTIONS" EXCLUDE
            PATTERN "HelpEngineScripts" EXCLUDE
            PATTERN "set-svn-properties.sh" EXCLUDE
            PATTERN "set_svn_properties.sh" EXCLUDE)
    if (WITH_MILITARY_ITAR AND
        EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/military_radios)
        file(GLOB mr_docs ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/military_radios/*)
        foreach (mr_doc ${mr_docs})
            if (mr_doc MATCHES "/\\.svn$")
                # skip
            elseif (IS_DIRECTORY ${mr_doc})
                install(DIRECTORY ${mr_doc} DESTINATION documentation
                    COMPONENT documentation
                    PATTERN ".svn" EXCLUDE
                    PATTERN "checkout-docs" EXCLUDE
                    PATTERN "military_radios" EXCLUDE
                    PATTERN "README" EXCLUDE
                    PATTERN "README-SVN-COMMIT-INSTRUCTIONS" EXCLUDE
                    PATTERN "HelpEngineScripts" EXCLUDE
                    PATTERN "set-svn-properties.sh" EXCLUDE
                    PATTERN "set_svn_properties.sh" EXCLUDE)
            else ()
                install(FILES ${mr_doc} DESTINATION documentation
                    COMPONENT documentation)
            endif ()
        endforeach ()
    endif ()
endif ()

include(InstallRequiredSystemLibraries)

# Include some convenience targets for running single-component installs,
# except on Visual Studio where it would clutter the solution with yet
# more small projects.
if (NOT (CMAKE_GENERATOR MATCHES "^Visual Studio"))
    get_cmake_property(component_list COMPONENTS)
    foreach (comp ${component_list})
        add_custom_target(install-${comp}
            COMMAND ${CMAKE_COMMAND} -DCOMPONENT=${comp} -P ./cmake_install.cmake
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(install-${comp}
            PROPERTIES FOLDER "Installer Components")
    endforeach ()
endif ()

# Create build.txt for build directory
string(TIMESTAMP build_txt_timestamp)
file(WRITE ${CMAKE_BINARY_DIR}/build.txt
    "User build of ${BUILD_TARGET} ${BUILD_TARGET_VERSION} configured at ${build_txt_timestamp}\n")
install(FILES ${CMAKE_BINARY_DIR}/build.txt DESTINATION .)
# for NDT builds, also needs to be in the separate ClassicGUI installer
# component of client-apps-installer
install(FILES ${CMAKE_BINARY_DIR}/build.txt DESTINATION . COMPONENT gui)

feature_summary(WHAT ALL)
