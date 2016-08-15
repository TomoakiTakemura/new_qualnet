option(WITH_UMTS "Build the UMTS library" ON)

if (WITH_UMTS)
  if (WITH_MUOS)
    message(SEND_ERROR "The MUOS and UMTS libraries cannot be compiled simultaneously")
  endif ()

  add_global_definitions(-DUMTS_LIB)
  add_global_include_dir(src)
  add_srcs(
    src/app_umtscall.cpp
    src/app_umtscall.h
    src/cellular_umts.cpp
    src/cellular_umts.h
    src/layer2_umts.cpp
    src/layer2_umts.h
    src/layer3_umts.cpp
    src/layer3_umts.h
    src/layer3_umts_ue.cpp
    src/layer3_umts_ue.h
    src/layer3_umts_nodeb.cpp
    src/layer3_umts_nodeb.h
    src/layer3_umts_rnc.cpp
    src/layer3_umts_rnc.h
    src/layer3_umts_gsn.cpp
    src/layer3_umts_gsn.h
    src/layer3_umts_hlr.cpp
    src/layer3_umts_hlr.h
    src/layer2_umts_pdcp.cpp
    src/layer2_umts_pdcp.h
    src/layer2_umts_rlc.cpp
    src/layer2_umts_rlc.h
    src/layer2_umts_mac.cpp
    src/layer2_umts_mac.h
    src/layer2_umts_mac_phy.cpp
    src/layer2_umts_mac_phy.h
    src/phy_umts.cpp
    src/phy_umts.h
    src/umts_constants.h
    src/umts_qos.h)
  add_scenario_dir(umts)
  install_clone(DIRECTORY gui COMPONENT gui)
  add_doxygen_inputs(src)
endif ()
