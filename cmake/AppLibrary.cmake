include_guard()

# Wrapper for add_library which enables common options we want for all
# libraries.
function(add_app_library name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "")
    add_library(${name} ${arg_UNPARSED_ARGUMENTS})

    target_compile_options(${name}
        PRIVATE
        -Wall
        -Wextra
        -Werror
    )
endfunction()
