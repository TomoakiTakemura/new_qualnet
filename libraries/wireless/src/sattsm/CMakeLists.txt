option(WITH_SATELLITE "Enable the Satellite library" ON)

if (WITH_SATELLITE)
  add_global_include_dir(.)
  add_global_definitions(-DADDON_SATTSM)
  add_srcs(
    analog.cpp
    analog.h
    phy_sattsm.cpp
    phy_sattsm.h
    afe-base.h
    afe-header.h
    antenna_parabolic.h
    digital.h
    lna.h
    mixer.h
    sattsm.h
    sspa.h
    twta.h
    viewpoint.h)
  add_scenario_dir(wireless/tsm)
  add_doxygen_inputs(.)
endif ()
