cmake_minimum_required(VERSION 3.22.1)

# TODO: Remove -Wno-unused-variable
# That warning is ignore for now because those are release config only issues
# caused by assert(). The fix is to migrate to CHECK/DCHECK, but that requires
# importing absl or libbase first.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-unused-variable")

add_library(accelerometergraph SHARED
            sensorgraph.cpp)

# Include libraries needed for accelerometergraph lib
target_link_libraries(accelerometergraph
                      android
                      GLESv2
                      log)
