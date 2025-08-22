cmake_minimum_required(VERSION 3.22.1)
project(NnSamplesSequence LANGUAGES CXX)

add_library(sequence
    SHARED
    sequence.cpp
    sequence_model.cpp
)

target_link_libraries(sequence
    # Link with libneuralnetworks.so for NN API
    neuralnetworks
    android
    log
)
