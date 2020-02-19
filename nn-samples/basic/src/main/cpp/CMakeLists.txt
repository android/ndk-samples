cmake_minimum_required(VERSION 3.4.1)

add_library(basic
            SHARED
            nn_sample.cpp
            simple_model.cpp)

target_link_libraries(basic

                      # Link with libneuralnetworks.so for NN API
                      neuralnetworks
                      android
                      log)
