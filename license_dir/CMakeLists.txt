if (WIN32)
    set(shlib_install_mode PROGRAMS)
else ()
    set(shlib_install_mode FILES)
endif ()

if (WIN32)
    install_clone(PROGRAMS ${BUILD_PLATFORM_BASE}/application/installanchorservice.exe)
    install_clone(${shlib_install_mode} ${BUILD_PLATFORM_BASE}/application/FNP_Act_Installer${CMAKE_SHARED_LIBRARY_SUFFIX})
elseif (UNIX)
    install_clone(PROGRAMS
        ${BUILD_PLATFORM_BASE}/application/install_fnp.sh
        ${BUILD_PLATFORM_BASE}/application/FNPLicensingService/FNPLicensingService)
endif ()

if (NOT BUILDING_LICENSE_APP)
    install(CODE "message(STATUS \"Installing LicenseApplication from license_dir binaries, platform ${BUILD_PLATFORM}...\")"
            COMPONENT license-application)
    install_clone(PROGRAMS ${BUILD_PLATFORM_BASE}/application/LicenseApplication${CMAKE_EXECUTABLE_SUFFIX}
                  COMPONENT license-application)
    install_clone(DIRECTORY ${BUILD_PLATFORM_BASE}/application/icons
                  COMPONENT license-application)
endif ()
if (NOT BUILDING_LICENSE_APP_BACKEND)
    install(CODE "message(STATUS \"Installing LicenseApplication backends from license_dir binaries, platform ${BUILD_PLATFORM}...\")"
            COMPONENT license-application)
    install_clone(PROGRAMS ${BUILD_PLATFORM_BASE}/application/NodeLockedLicense${CMAKE_EXECUTABLE_SUFFIX}
                           ${BUILD_PLATFORM_BASE}/application/ServerLicense${CMAKE_EXECUTABLE_SUFFIX}
                  COMPONENT license-application)
    install_clone(${shlib_install_mode}
        ${BUILD_PLATFORM_BASE}/application/FnpCommsSoap${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${BUILD_PLATFORM_BASE}/application/NodeLockedLicense_LibFNP${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${BUILD_PLATFORM_BASE}/application/ServerLicense_LibFNP${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMPONENT license-application)
    if (UNIX)
        install_clone(${shlib_install_mode}
            ${BUILD_PLATFORM_BASE}/application/lmflex_libFNP${CMAKE_SHARED_LIBRARY_SUFFIX}
            COMPONENT license-application)
    endif ()
endif ()
