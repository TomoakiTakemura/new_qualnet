if (WIN32)  # AGI interface is Windows only
    option(WITH_AGI_INTERFACE "Build the interface to AGI's STK" ON)

    if (WITH_AGI_INTERFACE)
        add_global_definitions(-DAGI_INTERFACE)
        add_global_include_dir(
            src
            src/STK9
            src/STK10
            src/STK11)
        add_srcs(
            src/STK9/agi_interface_id_stk9.cpp
            src/STK9/agi_interface_time_stk9.cpp
            src/STK9/agi_interface_signal_stk9.cpp
            src/STK9/agi_interface_mapping_stk9.cpp
            src/STK9/agi_interface_util_stk9.cpp
            src/STK9/agi_interface_util_stk9_create.cpp
            src/STK9/agi_interface_id_stk9.h
            src/STK9/agi_interface_mapping_stk9.h
            src/STK9/agi_interface_signal_stk9.h
            src/STK9/agi_interface_time_stk9.h
            src/STK9/agi_interface_util_stk9.h
            src/STK9/agi_interface_util_stk9_create.h
            src/STK9/AgStkCommUtil_v9.tlh
            src/STK10/agi_interface_id_stk10.cpp
            src/STK10/agi_interface_time_stk10.cpp
            src/STK10/agi_interface_signal_stk10.cpp
            src/STK10/agi_interface_mapping_stk10.cpp
            src/STK10/agi_interface_util_stk10.cpp
            src/STK10/agi_interface_util_stk10_create.cpp
            src/STK10/agi_interface_id_stk10.h
            src/STK10/agi_interface_mapping_stk10.h
            src/STK10/agi_interface_signal_stk10.h
            src/STK10/agi_interface_time_stk10.h
            src/STK10/agi_interface_util_stk10.h
            src/STK10/agi_interface_util_stk10_create.h
            src/STK10/AgStkCommUtil_v10.tlh
            src/STK11/agi_interface_id_stk11.cpp
            src/STK11/agi_interface_time_stk11.cpp
            src/STK11/agi_interface_signal_stk11.cpp
            src/STK11/agi_interface_mapping_stk11.cpp
            src/STK11/agi_interface_util_stk11.cpp
            src/STK11/agi_interface_util_stk11_create.cpp
            src/STK11/agi_interface_id_stk11.h
            src/STK11/agi_interface_mapping_stk11.h
            src/STK11/agi_interface_signal_stk11.h
            src/STK11/agi_interface_time_stk11.h
            src/STK11/agi_interface_util_stk11.h
            src/STK11/agi_interface_util_stk11_create.h
            src/agi_interface_util.cpp
            src/agi_interface.cpp
            src/agi_common_interfaces.h
            src/agi_interface.h
            src/agi_interface_id_impl.h
            src/agi_interface_mapping_impl.h
            src/agi_interface_signal_impl.h
            src/agi_interface_smart_ptr.h
            src/agi_interface_time_impl.h
            src/agi_interface_util.h
            src/agi_interface_util_impl.h
            src/agi_interface_util_impl2.h)
        add_doxygen_inputs(src src/STK9 src/STK10 src/STK11)
    endif ()
endif ()
