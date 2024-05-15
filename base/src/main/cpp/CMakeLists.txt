cmake_minimum_required(VERSION 3.22.1)
project(Base LANGUAGES CXX)

add_compile_options(-Wall -Werror -Wextra)

add_library(base
    STATIC
    logging.cpp
)

target_compile_features(base PRIVATE cxx_std_23)
target_compile_options(base PRIVATE -Wno-vla-cxx-extension)
target_include_directories(base PUBLIC include)
target_link_libraries(base PUBLIC log)
