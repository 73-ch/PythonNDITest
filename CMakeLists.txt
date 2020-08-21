cmake_minimum_required(VERSION 3.16)
project(python_ndi)

set(CMAKE_CXX_STANDARD 14)

set(PYTHON_EXECUTABLE /usr/local/bin/python3)

find_package(pybind11 REQUIRED)

include_directories(include)

link_directories(/usr/local/lib/x86_64-linux-gnu)

pybind11_add_module(pythonndi MODULE send.cpp)

target_link_libraries(pythonndi PRIVATE ndi)
