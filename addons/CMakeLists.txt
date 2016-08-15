if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/kernel)
    add_subdirectory(kernel)
else ()
    set(USE_FLEXNET ON CACHE INTERNAL "Must use FlexNet as private code is not present" FORCE)
endif ()

add_subdirectory(db)

if (UNIX AND    # CTDB is Linux only
    (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb7 OR
     EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb8))
    find_package(ZLIB)

    if (ZLIB_FOUND)
        option(WITH_CTDB "Build the CTDB terrain model" ON)
    else ()
        set(WITH_CTDB OFF)
    endif ()
else ()
    set(WITH_CTDB OFF)
endif ()

if (WITH_CTDB)

    option(WITH_CTDB "Build the CTDB terrain model" ON)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb7 AND
        EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb8)
        set(CTDB_VERSION 8 CACHE STRING "CTDB version to use (7 or 8)")
        set_property(CACHE CTDB_VERSION
            PROPERTY STRINGS "7;8")
        if (NOT ((CTDB_VERSION EQUAL 7) OR
                 (CTDB_VERSION EQUAL 8)))
            message(SEND_ERROR "Invalid CTDB_VERSION (must be 7 or 8)")
        endif ()
    elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb8)
        set(CTDB_VERSION 8)
    else ()
        set(CTDB_VERSION 7)
    endif ()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb7)
        add_subdirectory(ctdb7)
    endif ()
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ctdb8)
        add_subdirectory(ctdb8)
    endif ()

    add_feature_info(ctdb WITH_CTDB "CTDB (version ${CTDB_VERSION}) terrain model")

else ()

    add_feature_info(ctdb WITH_CTDB "CTDB terrain model")

endif ()
