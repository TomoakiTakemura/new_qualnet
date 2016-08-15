option(WITH_ADVANCED_WIRELESS "Build the Advanced Wireless library" ON)

if (WITH_ADVANCED_WIRELESS)
  add_global_definitions(-DADVANCED_WIRELESS_LIB)
  add_global_include_dir(src)
  add_srcs(
    src/dot16_backbone.cpp
    src/dot16_backbone.h
    src/mac_dot16_bs.cpp
    src/mac_dot16_bs.h
    src/mac_dot16.cpp
    src/mac_dot16.h
    src/mac_dot16_cs.cpp
    src/mac_dot16_cs.h
    src/mac_dot16e.h
    src/mac_dot16_phy.cpp
    src/mac_dot16_phy.h
    src/mac_dot16_qos.cpp
    src/mac_dot16_qos.h
    src/mac_dot16_sch.cpp
    src/mac_dot16_sch.h
    src/mac_dot16_ss.cpp
    src/mac_dot16_ss.h
    src/mac_dot16_tc.cpp
    src/mac_dot16_tc.h
    src/phy_dot16.cpp
    src/phy_dot16.h)
  add_scenario_dir(advanced_wireless)
  add_doxygen_inputs(src)
endif ()
