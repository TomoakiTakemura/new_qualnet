foreach (optional_contrib_model
         maodv)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${optional_contrib_model})
        add_subdirectory(${optional_contrib_model})
    endif ()
endforeach ()
