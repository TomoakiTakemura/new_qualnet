macro (split_install_args files_var install_args_var)
    set(split_install_args_in_install_args 0)
    set(${files_var})
    set(${install_args_var})
    foreach (arg ${ARGN})
        if (arg STREQUAL "COMPONENT" OR
            arg STREQUAL "PERMISSIONS" OR
            arg STREQUAL "CONFIGURATIONS" OR
            arg STREQUAL "RENAME" OR
            arg STREQUAL "OPTIONAL" OR
            arg STREQUAL "PATTERN" OR
            arg STREQUAL "REGEX" OR
            arg STREQUAL "USE_SOURCE_PERMISSIONS")
            set(split_install_args_in_install_args 1)
        endif ()
        if (split_install_args_in_install_args)
            list(APPEND ${install_args_var} "${arg}")
        else ()
            list(APPEND ${files_var} "${arg}")
        endif ()
    endforeach ()
endmacro ()

function (install_clone_aux basedir mode)
    string(LENGTH ${basedir} basedir_len)
    math(EXPR basedir_len_p1 "${basedir_len} + 1")
    split_install_args(install_files install_args ${ARGN})
    foreach (file ${install_files})
        get_filename_component(abspath ${file} ABSOLUTE)
        get_filename_component(abspath ${abspath} PATH)
        string(SUBSTRING ${abspath} 0 ${basedir_len_p1} abspath_head)
        if (abspath_head STREQUAL "${basedir}/")
            string(SUBSTRING ${abspath} ${basedir_len_p1} -1 abspath_tail)
            if (mode STREQUAL "DIRECTORY")
                install(${mode} ${file} DESTINATION ${abspath_tail}
                        ${install_args}
                        PATTERN ".svn" EXCLUDE)
            else ()
                install(${mode} ${file} DESTINATION ${abspath_tail}
                        ${install_args})
            endif ()
        else ()
            message(SEND_ERROR "install_clone: ${file} seems to be outside the
expected base directory ${basedir}")
        endif ()
    endforeach ()
endfunction ()

function (install_clone mode)
    install_clone_aux(${CMAKE_SOURCE_DIR} ${mode} ${ARGN})
endfunction ()

function (install_clone_bin mode)
    install_clone_aux(${CMAKE_BINARY_DIR} ${mode} ${ARGN})
endfunction ()
