cmake_minimum_required(VERSION 3.20)
project(BeamSense_Release)

option(CXX "enable C++ compilation" ON)
set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

add_library(BeamSense src/DataReader.cpp src/BeamSense.cpp)
target_link_libraries(BeamSense ${CMAKE_THREAD_LIBS_INIT})

add_executable(example example.cpp)
target_link_libraries(example BeamSense)
target_include_directories(example PUBLIC src)