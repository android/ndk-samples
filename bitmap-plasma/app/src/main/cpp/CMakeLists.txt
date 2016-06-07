cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(plasma_SRCS plasma.c)
add_library(plasma SHARED ${plasma_SRCS})

# Include libraries needed for plasma lib
target_link_libraries(plasma android log m jnigraphics)
