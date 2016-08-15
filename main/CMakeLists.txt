if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # suppress an error message that gets printed despite the FATAL_ERROR message
    cmake_minimum_required(VERSION 2.8)

    message(FATAL_ERROR "You must run CMake on the top-level directory of the source distribution, not on the main/ subdirectory.")
endif ()

add_core_srcs(
  external_socket.cpp
  memory.cpp
  WallClock.cpp
  gui_core.cpp)
add_srcs(
  app_util.cpp
  application.cpp
  atomic.cpp
  sliding_win.cpp
  circularbuffer.cpp
  dynamic.cpp
  external.cpp
  external_util.cpp
  geometry.cpp
  gui.cpp
  jlm.cpp  
  jlm_observer.cpp
  library_info.cpp
  list.cpp
  message.cpp
  node.cpp
  partition.cpp
  random.cpp
  stubs.cpp
  trace.cpp

  mac.cpp
  mobility.cpp
  proc-stats-db-controller.cpp
  terrain.cpp
  network.cpp
  stats.cpp
  stats_app.cpp
  stats_global.cpp
  stats_mac.cpp
  stats_net.cpp
  stats_phy.cpp
  stats_transport.cpp
  stats_queue.cpp
  timer_manager.cpp
  transport.cpp
  user.cpp
  interference.cpp
  spectrum.cpp
  phy_connectivity.cpp
  mimo_tgn_models.cpp)

if (IS_EXATA)
  add_srcs(socketlayer.cpp)
endif ()

add_doxygen_inputs(.)
if (NOT IS_EXATA)
    add_doxygen_excludes(socketlayer.cpp)
endif ()
