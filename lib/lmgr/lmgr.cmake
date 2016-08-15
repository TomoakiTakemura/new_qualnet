# FlexNet licensing libraries - only need separate libraries on Windows
# (on Linux, they're bundled into main.o.linux-*)

if (WIN32)
    if (USE_FLEXNET)
        target_link_libraries(${BUILD_TARGET}
            netapi32 comctl32 crypt32 wintrust
            ${CMAKE_CURRENT_LIST_DIR}/${BUILD_PLATFORM_BASE}/lmgr.lib)
    else ()
        # Don't need licensing library (note this is ONLY an option for
        # SCALABLE internal development)
    endif ()
endif ()
