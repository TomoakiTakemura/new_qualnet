add_subdirectory(obj)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src)
  add_subdirectory(src)
endif ()
