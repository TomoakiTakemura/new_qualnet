if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
    if (USE_FLEXNET)
        set(Boost_USE_STATIC_RUNTIME ON)
        set(Boost_USE_DEBUG_RUNTIME OFF)
    endif ()
endif ()
find_package(Boost 1.57 COMPONENTS thread system)

find_package(MariaDB)
find_package(SQLite3)

cmake_dependent_option(WITH_STATSDB "Build Statistics Database support" ON
                       "Boost_FOUND;MARIADB_FOUND;SQLITE3_FOUND" OFF)

if (WITH_STATSDB)
    add_global_system_include_dir_absolute(
        ${Boost_INCLUDE_DIRS}
        ${MARIADB_INCLUDE_DIR}
        ${SQLite3_INCLUDE_DIR})
    if (WIN32 AND USE_FLEXNET)
        # force using release libraries even in debug mode
        add_global_link_libs(${Boost_SYSTEM_LIBRARY_RELEASE} ${Boost_THREAD_LIBRARY_RELEASE})
    else ()
        add_global_link_libs(${Boost_LIBRARIES})
    endif ()

    add_global_link_libs(
        ${MARIADB_LIBRARY}
        ${SQLite3_LIBRARY})

    add_global_definitions(-DADDON_DB)
    add_global_include_dir(src)
    add_srcs(
        src/dbapi.cpp
        src/dbapi.h
        src/db.cpp
        src/db.h
        src/db_statsapi_bridge.cpp
        src/db_statsapi_bridge.h
        src/db-core.h
        src/db-mariadb.cpp
        src/db-mariadb.h
        src/db-sqlite3.cpp
        src/db-sqlite3.h)
    add_scenario_dir(statsdb)
    add_doxygen_inputs(src)
endif ()
