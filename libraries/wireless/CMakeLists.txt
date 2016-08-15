add_global_definitions(-DWIRELESS_LIB -DSATELLITE_LIB -DKERNEL_LITE)
add_global_include_dir(src src/olsrv2 src/shapelib src/shptoxml src/wifi)

# Expat needed by terrain_qualnet_urban_parser.cpp
find_package(EXPAT REQUIRED)
add_global_system_include_dir_absolute(${EXPAT_INCLUDE_DIRS})
add_global_link_libs(${EXPAT_LIBRARIES})

add_srcs(
  src/antenna.cpp
  src/antenna_global.cpp
  src/antenna_global.h
  src/antenna_patterned.cpp
  src/antenna_patterned.h
  src/antenna_steerable.cpp
  src/antenna_steerable.h
  src/antenna_switched.cpp
  src/antenna_switched.h
  src/cellular_layer3.cpp
  src/cellular_layer3.h
  src/dvcs.h
  src/mac_aloha.cpp
  src/mac_aloha.h
  src/mac_cellular.cpp
  src/mac_cellular.h
  src/mac_csma.cpp
  src/mac_csma.h
  src/wifi/mac_dot11-ap.cpp
  src/wifi/mac_dot11-ap.h
  src/wifi/mac_dot11.cpp
  src/wifi/mac_dot11.h
  src/wifi/mac_dot11-mgmt.cpp
  src/wifi/mac_dot11-mgmt.h
  src/wifi/mac_dot11-mib.cpp
  src/wifi/mac_dot11-mib.h
  src/wifi/mac_dot11-pc.cpp
  src/wifi/mac_dot11-pc.h
  src/wifi/mac_dot11-sta.cpp
  src/wifi/mac_dot11-sta.h
  src/wifi/mac_dot11_powersave_manager.cpp
  src/wifi/mac_dot11_powersave_manager.h
  src/wifi/mac_dot11-hcca.cpp
  src/wifi/mac_dot11-hcca.h
  src/wifi/mac_dot11_channelmanager.h
  src/wifi/mac_dot11_channelmanager.cpp
  src/wifi/mac_dot11_maccontroller.h
  src/wifi/mac_dot11s.cpp
  src/wifi/mac_dot11s.h
  src/wifi/mac_dot11s-frames.cpp
  src/wifi/mac_dot11s-frames.h
  src/wifi/mac_dot11s-hwmp.cpp
  src/wifi/mac_dot11s-hwmp.h
  src/wifi/mac_wave_dot11p.cpp
  src/wifi/mac_wave_dot11p.h
  src/wifi/mac_dot11n.cpp
  src/wifi/mac_dot11n.h
  src/wifi/mac_dot11_maccontroller.cpp
  src/wifi/mac_dot11_maccontroller.h
  src/wifi/mac_phy_802_11n.h
  src/wifi/mac_dot11ac.cpp
  src/wifi/mac_dot11ac.h
  src/wifi/phy_dot11ac.cpp
  src/wifi/phy_dot11ac.h
  src/wifi/phy_802_11_manager.cpp
  src/wifi/phy_802_11_manager.h
  src/wifi/mac_dot11_ac_manager.cpp
  src/wifi/mac_dot11_ac_manager.h
  src/wifi/phy_802_11.cpp
  src/wifi/phy_802_11.h
  src/wifi/phy_dot11a.cpp
  src/wifi/phy_dot11a.h
  src/wifi/phy_802_11p.h
  src/wifi/phy_802_11n.cpp
  src/wifi/phy_802_11n.h
  src/wifi/mimo-mapper.h
  src/mac_generic.cpp
  src/mac_generic.h
  src/mac_link_microwave.cpp
  src/mac_link_microwave.h
  src/mac_link_satellite.cpp
  src/mac_link_satellite.h
  src/mac_maca.cpp
  src/mac_maca.h
  src/mac_tdma.cpp
  src/mac_tdma.h
  src/mobility_group.cpp
  src/mobility_group.h
  src/mobility_waypoint.cpp
  src/mobility_waypoint.h
  src/multicast_odmrp.cpp
  src/multicast_odmrp.h
  src/network_ipv4_ndp.cpp
  src/network_ipv4_ndp.h
  src/network_neighbor_prot.cpp
  src/network_neighbor_prot.h
  src/energy_model.cpp
  src/energy_model.h
  src/phy.cpp
  src/battery_model.cpp
  src/battery_model.h
  src/phy_abstract.cpp
  src/phy_abstract.h
  src/phy_cellular.cpp
  src/phy_cellular.h
  src/propagation.cpp
  src/prop_itm.cpp
  src/prop_itm.h
  src/prop_itm_uarea.h
  src/prop_plmatrix.cpp
  src/prop_plmatrix.h
  src/routing_aodv.cpp
  src/routing_aodv.h
  src/manet_packet.cpp
  src/manet_packet.h
  src/routing_brp.cpp
  src/routing_brp.h
  src/routing_dsr.cpp
  src/routing_dsr.h
  src/routing_dymo.cpp
  src/routing_dymo.h
  src/routing_fisheye.cpp
  src/routing_fisheye.h
  src/routing_fsrl.cpp
  src/routing_fsrl.h
  src/routing_iarp.cpp
  src/routing_iarp.h
  src/routing_ierp.cpp
  src/routing_ierp.h
  src/routing_lar1.cpp
  src/routing_lar1.h
  src/routing_olsr-inria.cpp
  src/routing_olsr-inria.h
  src/routing_olsrv2_niigata.cpp
  src/routing_olsrv2_niigata.h
  src/routing_star.cpp
  src/routing_star.h
  src/routing_zrp.cpp
  src/routing_zrp.h
  src/terrain_cartesian.cpp
  src/terrain_cartesian.h
  src/terrain_dem.cpp
  src/terrain_dem.h
  src/terrain_dted.cpp
  src/terrain_dted.h
  src/terrain_qualnet_urban.cpp
  src/terrain_qualnet_urban.h
  src/terrain_qualnet_urban_parser.cpp
  src/terrain_qualnet_urban_parser.h
  src/terrain_esri_shp.cpp
  src/terrain_esri_shp.h
  src/terrain_esri_shp_main.cpp
  src/terrain_esri_shp_main.h
  src/terrain_esri_shp_shared.cpp
  src/terrain_esri_shp_shared.h

  src/olsrv2/olsr_common.h
  src/olsrv2/olsr_types.h
  src/olsrv2/def.h
  src/olsrv2/olsr_protocol.h
  src/olsrv2/proc_rcv_msg_set.cpp
  src/olsrv2/proc_rcv_msg_set.h
  src/olsrv2/proc_proc_set.cpp
  src/olsrv2/proc_proc_set.h
  src/olsrv2/proc_forward_set.cpp
  src/olsrv2/proc_forward_set.h
  src/olsrv2/proc_relay_set.cpp
  src/olsrv2/proc_relay_set.h
  src/olsrv2/pktbuf.cpp
  src/olsrv2/pktbuf.h
  src/olsrv2/proc_link_set.cpp
  src/olsrv2/proc_link_set.h
  src/olsrv2/proc_2neigh_set.cpp
  src/olsrv2/proc_2neigh_set.h
  src/olsrv2/proc_mpr_set.cpp
  src/olsrv2/proc_mpr_set.h
  src/olsrv2/proc_mpr_selector_set.cpp
  src/olsrv2/proc_mpr_selector_set.h
  src/olsrv2/proc_adv_neigh_set.cpp
  src/olsrv2/proc_adv_neigh_set.h
  src/olsrv2/proc_topology_set.cpp
  src/olsrv2/proc_topology_set.h
  src/olsrv2/proc_routing_set.cpp
  src/olsrv2/proc_routing_set.h
  src/olsrv2/proc_attach_network_set.cpp
  src/olsrv2/proc_attach_network_set.h
  src/olsrv2/proc_assn_history_set.cpp
  src/olsrv2/proc_assn_history_set.h
  src/olsrv2/proc_symmetric_neighbor_set.cpp
  src/olsrv2/proc_symmetric_neighbor_set.h
  src/olsrv2/address_compress.cpp
  src/olsrv2/address_compress.h
  src/olsrv2/tlv_compress.cpp
  src/olsrv2/tlv_compress.h
  src/olsrv2/parse_address.cpp
  src/olsrv2/parse_address.h
  src/olsrv2/parse_tlv.cpp
  src/olsrv2/parse_tlv.h
  src/olsrv2/generate_msg.cpp
  src/olsrv2/generate_msg.h
  src/olsrv2/parse_msg.cpp
  src/olsrv2/parse_msg.h
  src/olsrv2/proc_recv_packet.cpp
  src/olsrv2/proc_recv_packet.h
  src/olsrv2/packet.cpp
  src/olsrv2/packet.h
  src/olsrv2/mantissa.cpp
  src/olsrv2/mantissa.h
  src/olsrv2/olsr.cpp
  src/olsrv2/olsr.h
  src/olsrv2/olsr_list.cpp
  src/olsrv2/olsr_list.h
  src/olsrv2/olsr_hash_list.cpp
  src/olsrv2/olsr_hash_list.h
  src/olsrv2/olsr_util.cpp
  src/olsrv2/olsr_util.h
  src/olsrv2/olsr_util_inline.h
  src/olsrv2/olsr_debug.cpp
  src/olsrv2/olsr_debug.h
  src/olsrv2/olsr_conf.cpp
  src/olsrv2/olsr_conf.h
  src/olsrv2/proc_neighbor_address_association_set.cpp
  src/olsrv2/proc_neighbor_address_association_set.h

  src/shptoxml/shptoxml_main.cpp
  src/shptoxml/shptoxml_main.h
  src/shptoxml/shptoxml_shared.cpp
  src/shptoxml/shptoxml_shared.h
  src/shapelib/dbfopen.c
  src/shapelib/shpopen.c
  src/shapelib/shapefil.h
  src/mac_satellite_bentpipe.cpp
  src/mac_satellite_bentpipe.h
  src/phy_satellite_rsv.cpp
  src/phy_satellite_rsv.h
  src/nameserver.cpp
  src/util_nameservice.h
  src/util_constants.h)
add_scenario_dir(
  wireless/aloha
  wireless/antenna
  wireless/aodv
  wireless/battery-models
  wireless/wifi/dot11
  wireless/wifi/dot11ac
  wireless/wifi/dot11e
  wireless/wifi/dot11n
  wireless/wifi/dot11p
  wireless/wifi/dot11s
  wireless/wifi/interoperability
  wireless/dsr
  wireless/dymo
  wireless/energy-models
  wireless/group-mobility
  wireless/lanmar
  wireless/microwave
  wireless/mixed-networks
  wireless/named-channels
  wireless/odmrp
  wireless/olsr-inria
  wireless/olsrv2-niigata
  wireless/pathloss-matrix
  wireless/phy-abstract
  wireless/rsv
  wireless/star
  wireless/tdma
  wireless/terrain-dem
  wireless/terrain-dted
  wireless/terrain-feature
  wireless/weather
  wireless/zrp)
add_doxygen_inputs(src src/oslrv2 src/shptoxml src/shapelib)

add_subdirectory(src/ane)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/sattsm)
  add_subdirectory(src/sattsm)
endif ()

install_clone(DIRECTORY gui COMPONENT gui)

# Building shptoxml utility
add_executable(shptoxml
  src/shptoxml/shptoxml.cpp
  src/shptoxml/shptoxml.h
  src/shptoxml/shptoxml_main.cpp
  src/shptoxml/shptoxml_main.h
  src/shptoxml/shptoxml_shared.cpp
  src/shptoxml/shptoxml_shared.h
  src/shapelib/dbfopen.c
  src/shapelib/shpopen.c
  src/shapelib/shapefil.h)
set_target_properties(shptoxml PROPERTIES
  COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS}"
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/shptoxml;${CMAKE_CURRENT_SOURCE_DIR}/src/shapelib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
  FOLDER "Utilities")

install(TARGETS shptoxml RUNTIME DESTINATION bin)

# Register radio_range utility to link against simlib
add_utility_target_include(${CMAKE_CURRENT_SOURCE_DIR}/radio_range.cmake)

add_feature_info(wireless ON "Basic wireless networking models")
