cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -Werror -DANDROID_NDK -DDISABLE_IMPORTGL")
set(sanangeles_SRCS app-android.c
                    demo.c
                    importgl.c)
add_library(sanangeles SHARED ${sanangeles_SRCS})

# Include libraries needed for sanangeles lib
target_link_libraries(sanangeles android log m dl GLESv1_CM)
