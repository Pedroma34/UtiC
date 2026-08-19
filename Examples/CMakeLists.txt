cmake_minimum_required(VERSION 3.22)
project(Testbed LANGUAGES C)
set(CMAKE_C_STANDARD 11 REQUIRED)

set(
    SOURCES
    "testbed.c"
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    UtiC
)
