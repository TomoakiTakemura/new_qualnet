# Build proxy_sim utility; we do this in a file included from the top-level
# CMakeLists.txt file instead of in interfaces/multi-gui/CMakeLists.txt
# so that we can get the final values of ALL_INCLUDES, etc., and also
# make sure we build after simlib is ready.

add_library(proxy-sim_stubs ${CMAKE_CURRENT_LIST_DIR}/src/proxy_sim_intf_stubs.cpp)
set_target_properties(proxy-sim_stubs
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
  FOLDER "Utilities/proxy-sim")
add_executable(proxy-sim
    ${CMAKE_CURRENT_LIST_DIR}/src/proxy_sim_intf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/proxy_atomic.cpp)
target_link_libraries(proxy-sim kernel_core simlib_core proxy-sim_stubs)
if (UNIX)
    target_link_libraries(proxy-sim pthread)
elseif (WIN32)
    target_link_libraries(proxy-sim ws2_32 ${PTH_WIN32_LIBRARY})
endif ()
set_target_properties(proxy-sim
  PROPERTIES COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
             RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin
             RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
             FOLDER "Utilities/proxy-sim")
install(TARGETS proxy-sim RUNTIME DESTINATION bin COMPONENT gui)
