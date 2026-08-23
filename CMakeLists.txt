cmake_minimum_required(VERSION 3.22)
project(Util)

include(CTest)

option(UTIC_BUILD_BENCHMARKS "Build UtiC performance benchmarks" OFF)

add_subdirectory(UtiC)
add_subdirectory(Examples)

if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()

if(UTIC_BUILD_BENCHMARKS)
    add_subdirectory(Benchmarks)
endif()
