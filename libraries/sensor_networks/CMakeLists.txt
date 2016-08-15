option(WITH_SENSOR_NETWORKS "Build the Sensor Networks library" ON)

if (WITH_SENSOR_NETWORKS)
  add_global_definitions(-DSENSOR_NETWORKS_LIB)
  add_global_include_dir(src)
  add_srcs(
    src/sscs_802_15_4.cpp
    src/sscs_802_15_4.h
    src/mac_802_15_4_transac.cpp
    src/mac_802_15_4_transac.h
    src/csma_802_15_4.cpp
    src/csma_802_15_4.h
    src/phy_802_15_4.cpp
    src/phy_802_15_4.h
    src/mac_802_15_4.cpp
    src/mac_802_15_4.h
    src/mac_802_15_4_cmn.h
    src/app_zigbeeApp.cpp
    src/app_zigbeeApp.h)
  add_scenario_dir(sensor_networks)
  add_doxygen_inputs(src)
endif ()
