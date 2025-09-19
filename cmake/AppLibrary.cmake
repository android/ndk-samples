include_guard()

# Wrapper for add_library which enables common options we want for all
# libraries.
function(add_app_library name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "NO_VERSION_SCRIPT" "" "")
    add_library(${name} ${arg_UNPARSED_ARGUMENTS})

    target_compile_options(${name}
        PRIVATE
        -Wall
        -Wextra
        -Werror
    )

    if(NOT arg_NO_VERSION_SCRIPT)
        target_link_options(${name}
            PRIVATE
            -Wl,--no-undefined-version
            -Wl,--version-script,${CMAKE_SOURCE_DIR}/lib${name}.map.txt
        )

        set_target_properties(${name}
            PROPERTIES
            LINK_DEPENDS ${CMAKE_SOURCE_DIR}/lib${name}.map.txt
        )
    endif()
endfunction()
