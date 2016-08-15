find_package(TIREM)
cmake_dependent_option(WITH_TIREM "Build the TIREM propagation model" ON
                       TIREM_FOUND OFF)

if (WITH_TIREM)
    add_global_definitions(-DTIREM_LIB)
    add_global_system_include_dir_absolute(${TIREM_INCLUDE_DIR})
    add_global_include_dir(src)

    add_srcs(
        src/prop_tirem.cpp
        src/prop_tirem.h)

    # copy library into addons/tirem
    file(INSTALL ${TIREM_LIBRARY}
         DESTINATION ${CMAKE_BINARY_DIR}/addons/tirem
         USE_SOURCE_PERMISSIONS)
endif ()

add_feature_info(tirem WITH_TIREM "TIREM propagation model")
