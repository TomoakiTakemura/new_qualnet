include(CMakeDependentOption)

option(WITH_SOPSVOPS_INTERFACE "Build the SopsVops interface" ON)
CMAKE_DEPENDENT_OPTION(SOPS_DEBUG "Enable SOPS debugging" OFF
                        "WITH_SOPSVOPS_INTERFACE" OFF)
CMAKE_DEPENDENT_OPTION(SOPS_EXTRA_DEBUG "Enable extra SOPS debugging" OFF
                        "WITH_SOPSVOPS_INTERFACE" OFF)
mark_as_advanced(SOPS_DEBUG SOPS_EXTRA_DEBUG)

if (WITH_SOPSVOPS_INTERFACE)
  add_global_definitions(-DSOPSVOPS_INTERFACE)
  if (SOPS_EXTRA_DEBUG)
    add_global_definitions(-DSOPS_DEBUG -DSOPS_EXTRA_DEBUG)
  elseif (SOPS_DEBUG)
    add_global_definitions(-DSOPS_DEBUG)
  endif ()

  add_global_include_dir(src)
  add_srcs(
    src/Property.h
    src/RpcInterface.h
    src/spinterface.h
    src/ops_util.cpp
    src/ops_util.h
    src/SopsProperty.cpp
    src/SopsProperty.h
    src/SopsRpcInterface.cpp
    src/SopsRpcInterface.h
    src/SPConnection.cpp
    src/SPConnection.h)
  if (IS_EXATA)
    add_scenario_dir(scenario_player/PlayerPalette)
    if (WITH_CYBER)
      add_scenario_dir(scenario_player/SantaBarbara)
    endif ()
  endif ()
  add_doxygen_inputs(src)
endif ()

# NDT interface is only supported on a subset of our platforms
if (WIN32)
    set(NDT_PLATFORM_SUPPORTED 1)
elseif (UNIX)
    # On Linux, 64-bit only.
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(NDT_PLATFORM_SUPPORTED 0)
    elseif (BUILD_PLATFORM MATCHES "^linux-x64-glibc-([0-9.]+)-gcc-([0-9.]+)$")
        # And due to binary requirements of libloggingapi.so, can't build
        # NDT interface on older platforms than Ubuntu 12.04 LTS.
        set(glibc_version ${CMAKE_MATCH_1})
        set(gcc_version ${CMAKE_MATCH_2})
        if (glibc_version VERSION_LESS "2.15" OR
            gcc_version VERSION_LESS "4.6")
            set(NDT_PLATFORM_SUPPORTED 0)
        else ()
            set(NDT_PLATFORM_SUPPORTED 1)
        endif ()
    else ()
        set(NDT_PLATFORM_SUPPORTED 0)
    endif ()
endif ()

if (BUILD_TARGET STREQUAL "ndt")
    set(def_WITH_NDT_INTERFACE ON)
else ()
    set(def_WITH_NDT_INTERFACE OFF)
endif ()

CMAKE_DEPENDENT_OPTION(WITH_NDT_INTERFACE "Build the NDT interface" ${def_WITH_NDT_INTERFACE}
    "IS_EXATA;NDT_PLATFORM_SUPPORTED" OFF)

if (WITH_NDT_INTERFACE)
  add_external_lib(zmq)
  add_global_definitions(-DNDT_INTERFACE)
  add_global_definitions(-DMULTI_GUI_INTERFACE)
  add_global_definitions(-DNO_DELAYED_CALLBACK)

   add_global_include_dir(src)
   add_srcs(
     src/SopsRpcInterface.cpp
     src/SopsRpcInterface.h)

  add_utility_target_include(${CMAKE_CURRENT_SOURCE_DIR}/ndt-interface.cmake)
endif ()
