foreach (optional_interface
    vrlink
    sopsvops
    socket-interface
    multi-gui
    agi
    interfacetutorial)
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${optional_interface})
    add_subdirectory(${optional_interface})
  endif ()
endforeach ()

add_subdirectory(uuid)

if (IS_EXATA)
  add_subdirectory(lib-emulation)
  add_subdirectory(ipne)
  add_subdirectory(auto-ipne)
  add_subdirectory(upa)
  add_subdirectory(gateway)
  add_subdirectory(pas)
  add_subdirectory(net-snmp)
  add_subdirectory(openSSL)
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ndt)
    add_subdirectory(ndt)
  endif ()
endif ()
