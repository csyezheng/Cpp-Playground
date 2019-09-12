cmake_minimum_required(VERSION 3.14)
project(system_io)

set(CMAKE_CXX_STANDARD 14)

# includes
set(CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
        ${CMAKE_MODULE_PATH})

set(TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SYSTEM_IO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/system_io")

find_package(Glog REQUIRED)
find_package(gflags CONFIG QUIET)
if (gflags_FOUND)
    message(STATUS "Found gflags from package config")
    message(STATUS "gflags_CONFIG=${gflags_CONFIG}")
else ()
    find_package(Gflags REQUIRED)
endif ()
find_package(Threads REQUIRED)


# This let project headers can be included with full path
include_directories(${TOP_DIR})

add_subdirectory(system_io)