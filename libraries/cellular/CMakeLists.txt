option(WITH_CELLULAR "Build the Cellular library" ON)

if (WITH_CELLULAR)
  add_global_definitions(-DCELLULAR_LIB)
  add_global_include_dir(src)
  add_srcs(
    src/cellular_abstract.h
    src/cellular_gsm.h
    src/app_cellular_abstract.cpp
    src/app_cellular_abstract.h
    src/cellular_abstract_layer3.cpp
    src/cellular_abstract_layer3.h
    src/layer3_gsm.cpp
    src/layer3_gsm.h
    src/mac_cellular_abstract.cpp
    src/mac_cellular_abstract.h
    src/mac_gsm.cpp
    src/mac_gsm.h
    src/phy_gsm.cpp
    src/phy_gsm.h
    src/user_profile_parser.cpp
    src/user_profile_parser.h
    src/user_trafficpattern_parser.cpp
    src/user_trafficpattern_parser.h)
  add_scenario_dir(cellular)
  install_clone(DIRECTORY gui COMPONENT gui)
  add_doxygen_inputs(src)
endif ()
