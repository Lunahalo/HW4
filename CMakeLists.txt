cmake_minimum_required(VERSION 3.3)
project(HW4)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

set(SOURCE_FILES HW4.c HW4.h)
add_executable(HW4 ${SOURCE_FILES})