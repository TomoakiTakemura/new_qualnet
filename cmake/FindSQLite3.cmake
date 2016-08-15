# Look for the header file.
find_path(SQLite3_INCLUDE_DIR NAMES sqlite3.h)

# Look for the library.
find_library(SQLite3_LIBRARY NAMES sqlite3 libsqlite3)

# Find version string.
if (SQLite3_INCLUDE_DIR AND EXISTS "${SQLite3_INCLUDE_DIR}/sqlite3.h")
    file(STRINGS "${SQLite3_INCLUDE_DIR}/sqlite3.h" sqlite3_version_str
         REGEX "^#define[\t ]+SQLITE_VERSION[\t ]+\".*\"")
    string(REGEX REPLACE "^#define[\t ]+SQLITE_VERSION[\t ]+\"(.*)\".*"
           "\\1" SQLite3_VERSION "${sqlite3_version_str}")
endif ()

# handle the find_package args and set PCRE_FOUND
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQLite3
    REQUIRED_VARS SQLite3_LIBRARY SQLite3_INCLUDE_DIR
    VERSION_VAR SQLite3_VERSION)

mark_as_advanced(SQLite3_INCLUDE_DIR SQLite3_LIBRARY)
