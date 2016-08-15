# Build radio_range utility; we do this in a file included from the top-level
# CMakeLists.txt file instead of in libraries/wireless/CMakeLists.txt
# so that we can get the final values of ALL_INCLUDES, etc., and also
# make sure we build after simlib is ready.

add_executable(radio_range ${CMAKE_CURRENT_LIST_DIR}/src/prop_range.cpp)
target_link_libraries(radio_range ${ALL_LINK_LIBS})
if (USE_MPI AND MPI_CXX_LIBRARIES)
    target_link_libraries(radio_range ${MPI_CXX_LIBRARIES})
endif ()
set_target_properties(radio_range
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
             RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
             FOLDER "Utilities")
if (USE_MPI AND MPI_CXX_LINK_FLAGS)
    set_target_properties(radio_range
        PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
endif ()

install(TARGETS radio_range RUNTIME DESTINATION bin)
