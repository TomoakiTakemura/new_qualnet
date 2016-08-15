option(WITH_MULTI_GUI_INTERFACE "Build the Multi Gui interface" ON)

if (WITH_MULTI_GUI_INTERFACE)
  add_global_definitions(-DMULTI_GUI_INTERFACE)
  add_global_include_dir(src)
  add_srcs(
    src/multi_gui_interface.cpp
    src/multi_gui_interface.h)
  add_utility_target_include(${CMAKE_CURRENT_SOURCE_DIR}/proxy-sim.cmake)
  add_doxygen_inputs(src)
endif ()

add_feature_info(multi-gui WITH_MULTI_GUI_INTERFACE "Provide a proxy to allow multiple legacy GUIs to connect to a single simulator")
