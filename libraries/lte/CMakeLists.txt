include(CMakeDependentOption)

option(WITH_LTE "Build the LTE (4G) library" ON)
CMAKE_DEPENDENT_OPTION(LTE_LOGGING "Enable LTE debugging log" OFF
                       "WITH_LTE" OFF)
CMAKE_DEPENDENT_OPTION(LTE_LOGGING_VALIDATION "Enable LTE validation log" OFF
                       "WITH_LTE" OFF)
mark_as_advanced(LTE_LOGGING LTE_LOGGING_VALIDATION)

if (WITH_LTE)
  add_global_definitions(-DLTE_LIB)
  if (LTE_LOGGING_VALIDATION)
    add_global_definitions(-DLTE_LIB_LOG -DLTE_LIB_VALIDATION_LOG)
  elseif (LTE_LOGGING)
    add_global_definitions(-DLTE_LIB_LOG)
  endif ()

  add_global_include_dir(src)
  add_srcs(
    src/lte_common.cpp
    src/lte_common.h
    src/lte_harq.cpp
    src/lte_harq.h
    src/lte_rrc_config.cpp
    src/lte_rrc_config.h
    src/log_lte.cpp
    src/log_lte.h
    src/phy_lte.cpp
    src/phy_lte.h
    src/phy_lte_establishment.cpp
    src/phy_lte_establishment.h
    src/phy_rxmodel_lte.cpp
    src/phy_rxmodel_lte.h
    src/layer2_lte.cpp
    src/layer2_lte.h
    src/layer2_lte_establishment.cpp
    src/layer2_lte_establishment.h
    src/layer2_lte_mac.cpp
    src/layer2_lte_mac.h
    src/layer2_lte_pdcp.cpp
    src/layer2_lte_pdcp.h
    src/layer2_lte_rlc.cpp
    src/layer2_lte_rlc.h
    src/layer2_lte_sch.cpp
    src/layer2_lte_sch.h
    src/layer2_lte_sch_roundrobin.cpp
    src/layer2_lte_sch_roundrobin.h
    src/layer2_lte_sch_pf.cpp
    src/layer2_lte_sch_pf.h
    src/layer2_lte_sch_ue_default.cpp
    src/layer2_lte_sch_ue_default.h
    src/layer3_lte.cpp
    src/layer3_lte.h
    src/layer3_lte_filtering.cpp
    src/layer3_lte_filtering.h
    src/layer3_lte_measurement.cpp
    src/layer3_lte_measurement.h
    src/epc_lte.cpp
    src/epc_lte.h
    src/epc_lte_app.cpp
    src/epc_lte_app.h
    src/epc_lte_common.h
    src/matrix_calc.cpp
    src/matrix_calc.h
    src/lte_transport_block_size_table.h)
  add_scenario_dir(lte)
  add_doxygen_inputs(src)

  # Simulator requires LTE data files at run time, so for an out-of-source
  # build, copy data/modulation/lte to the build directory
  if (NOT (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR))
    file(COPY "${CMAKE_SOURCE_DIR}/data/modulation/lte"
         DESTINATION "${CMAKE_BINARY_DIR}/data/modulation/"
         PATTERN ".svn" EXCLUDE)
  endif ()
endif (WITH_LTE)
