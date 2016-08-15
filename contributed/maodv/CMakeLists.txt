option(WITH_CONTRIB_MAODV "Build the contributed MAODV model" OFF)

if (WITH_CONTRIB_MAODV)
    add_global_definitions(-DADDON_MAODV)
    add_global_include_dir(.)
    add_srcs(
        multicast_maodv.cpp
        multicast_maodv.h)
    add_doxygen_inputs(.)
endif ()

add_feature_info(contrib_maodv WITH_CONTRIB_MAODV "Contributed MAODV model")
