cmake_minimum_required(VERSION 2.8.4)
project(opencl_example)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS  "-g -w -std=c++11")
set(CMAKE_CXX_FLAGS "-std=c++11 -g ")


include_directories("/usr/local/cuda/include")

add_executable(opencl_example ./hello.cpp)
target_link_libraries(opencl_example "/usr/local/cuda/lib64/libOpenCL.so.1.0")


