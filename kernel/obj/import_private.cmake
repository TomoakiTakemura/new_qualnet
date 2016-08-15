add_library(private STATIC IMPORTED)
set_target_properties(private
    PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/${BUILD_TARGET}-private-${BUILD_PLATFORM}${MPI_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

macro (private_link)
    set_target_properties(private
        PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${ARGN}")
endmacro ()
