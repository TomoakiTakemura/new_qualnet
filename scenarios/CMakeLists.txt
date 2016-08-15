foreach (dir ${ALL_SCENARIO_DIRS} vrf interfacetutorial dis hla)
    get_filename_component(dir_base "${dir}" DIRECTORY)
    install(DIRECTORY ${dir} DESTINATION scenarios/${dir_base}
            COMPONENT scenarios
            PATTERN "*.out" EXCLUDE
            PATTERN "*.stat" EXCLUDE
            PATTERN "*.trace" EXCLUDE
            PATTERN "*.dif" EXCLUDE
            PATTERN "*.mtslog" EXCLUDE
            PATTERN "*.docx" EXCLUDE
            PATTERN "tcptrace.asc" EXCLUDE
            PATTERN "tcptrace.dmp" EXCLUDE
            PATTERN "SOCKET-INTERFACE_*" EXCLUDE
            PATTERN "CommsDemoLink16" EXCLUDE)
    install(DIRECTORY ${dir} DESTINATION scenarios/${dir_base}
            COMPONENT scenarios
            FILES_MATCHING REGEX "\\.expected(\\.linux|)\\.(stat|trace)$"
            PATTERN "SOCKET-INTERFACE_*" EXCLUDE
            PATTERN "CommsDemoLink16" EXCLUDE)
endforeach ()

find_package(Perl)
if (PERL_FOUND)
    set(exec_params -simexec $<TARGET_FILE:${BUILD_TARGET}>)
    if (WITH_SOCKET_INTERFACE)
        list(APPEND exec_params -mtsexec $<TARGET_FILE:mts-socket>)
    endif ()

    option(TEST_JUNIT_COMPAT_NAMES
           "Set to use JUnit compatible test names, for example if you intend to use the test results in Jenkins"
           OFF)
    set(TEST_NAME_PREFIX "" CACHE STRING
        "Set to an optional prefix to add to test names, for example to distinguish between multiple test runs in Jenkins test results")
    set(TEST_TIMEOUT "3600" CACHE STRING
        "Timeout for each test case (in seconds)")
    mark_as_advanced(TEST_JUNIT_COMPAT_NAMES TEST_NAME_PREFIX TEST_TIMEOUT)

    macro (add_regression_test scen)
        set(testname ${scen})
        if (TEST_JUNIT_COMPAT_NAMES)
            string(REPLACE "." "_" testname "${testname}")
            string(REPLACE "/" "." testname "${testname}")
        endif ()
        if (TEST_NAME_PREFIX)
            if (TEST_JUNIT_COMPAT_NAMES)
                set(testname "${TEST_NAME_PREFIX}.${testname}")
            else ()
                set(testname "${TEST_NAME_PREFIX}/${testname}")
            endif ()
        endif ()
        add_test(NAME ${testname} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 COMMAND ${PERL_EXECUTABLE} "${CMAKE_BINARY_DIR}/bin/runtestcase"
                     ${exec_params} "${scen}")
        set_tests_properties(${testname} PROPERTIES
            TIMEOUT ${TEST_TIMEOUT})
    endmacro ()
else ()
    if (WIN32)
        message(WARNING "Could not find Perl so cannot run tests -- try installing Cygwin")
    else ()
        message(WARNING "Could not find Perl so cannot run tests")
    endif ()
    macro (add_regression_test scen)
    endmacro ()
endif ()

set(norun_scenarios
    samples/AguaDulce/AguaDulce.config
    samples/BlackStar/BlackStar.config)

foreach (dir ${ALL_SCENARIO_DIRS})
    file(GLOB_RECURSE config_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.config")
    foreach (scen ${config_files})
        list(FIND norun_scenarios ${scen} norun_index)
        if (norun_index EQUAL -1)
            add_regression_test(${scen})
        endif ()
    endforeach ()
endforeach ()

add_feature_info(testsuite PERL_FOUND "Run the regression testsuite with \"make test\"")
