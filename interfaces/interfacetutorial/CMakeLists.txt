option(WITH_INTERFACETUTORIAL "Build the interface tutorial" OFF)

if (WITH_INTERFACETUTORIAL)
    add_global_definitions(-DTUTORIAL_INTERFACE)
    add_global_include_dir(src)
    add_srcs(
        src/interfacetutorial.cpp
        src/interfacetutorial.h
        src/interfacetutorial_app.cpp
        src/interfacetutorial_app.h)
    add_doxygen_inputs(src)

    add_executable(tutorialtester src/tutorialtester.cpp)
    if (WIN32)
        target_link_libraries(tutorialtester ws2_32)
    endif ()
endif ()

add_feature_info(interfacetutorial WITH_INTERFACETUTORIAL "Interface tutorial")
