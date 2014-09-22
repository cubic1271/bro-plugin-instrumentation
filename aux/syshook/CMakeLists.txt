cmake_minimum_required(VERSION 2.6.3)

project(syshooks)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

link_libraries("-ldl")

add_library(syshook-io SHARED src/syshooks/syshook-io.cc)
add_library(syshook-malloc SHARED src/syshooks/syshook-malloc.cc)


