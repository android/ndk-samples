cmake_minimum_required(VERSION 3.22.1)
project(BitmapPlasma LANGUAGES C)

add_compile_options(-Wall -Wextra -Werror)

add_library(plasma SHARED plasma.c)

# Include libraries needed for plasma lib
target_link_libraries(plasma
    android
    jnigraphics
    log
    m
)
