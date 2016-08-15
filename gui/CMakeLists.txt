if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
    add_subdirectory(src)
elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../gui/src/CMakeLists.txt)
    add_subdirectory(../../gui/src src)
endif ()

install_clone(FILES devices/user.router-models
              COMPONENT gui)
# these files are also needed by synch utility
install_clone(FILES devices/builtin.vrlink devices/user.router-models
              COMPONENT federation-interfaces)

# Put gui/build_target after gui/src so that it can pick up selected
# GUI addons if building the Qt GUI.
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build_target)
    add_subdirectory(build_target)
else ()
    configure_file(
        ${CMAKE_SOURCE_DIR}/bin/runtestcase
        ${CMAKE_BINARY_DIR}/bin/runtestcase
        COPYONLY)
    install_clone(PROGRAMS
        ${CMAKE_SOURCE_DIR}/bin/runtestcase)
    install_clone(PROGRAMS
        ${CMAKE_SOURCE_DIR}/bin/setupRemoteExec.sh
        ${CMAKE_SOURCE_DIR}/bin/${BUILD_TARGET}.remote.sh
        COMPONENT gui)
    if (WIN32)
        install_clone(PROGRAMS
            ${CMAKE_SOURCE_DIR}/bin/${BUILD_TARGET}.remote.bat
            ${CMAKE_SOURCE_DIR}/bin/${gui_name}.exe
            COMPONENT gui)
    else ()
        install_clone(PROGRAMS ${CMAKE_SOURCE_DIR}/bin/${gui_name}
                      COMPONENT gui)
    endif ()
endif ()
