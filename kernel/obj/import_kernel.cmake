add_library(kernel_core STATIC IMPORTED)
set_target_properties(kernel_core
    PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/${BUILD_TARGET}-kernel_core-${BUILD_PLATFORM}${MPI_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(kernel STATIC IMPORTED)
set_target_properties(kernel
    PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/${BUILD_TARGET}-kernel-${BUILD_PLATFORM}${MPI_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

macro (kernel_link)
    set_target_properties(kernel
        PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${ARGN}")
endmacro ()
