cmake_minimum_required(VERSION 3.4.1)

add_library(nn_sample
            SHARED
            nn_sample.cpp
            simple_model.cpp)

target_link_libraries(nn_sample

                      # Link with libneuralnetworks.so for NN API
                      neuralnetworks
                      android
                      log)

