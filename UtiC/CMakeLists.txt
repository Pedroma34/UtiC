cmake_minimum_required(VERSION 3.22)
project(UtiC LANGUAGES C)
set(CMAKE_C_STANDARD 11 REQUIRED)

set(
    SOURCES
    #core
    "src/core/event_bus.c"
    #memory
    "src/memory/allocator.c"
    "src/memory/allocator_counter.c"
    "$<$<PLATFORM_ID:Windows>:src/platform/win32/allocator_system.c>"
    "$<$<PLATFORM_ID:Linux>:src/platform/linux/allocator_system.c>"
    "src/memory/arena.c"
    #containers
    "src/containers/darray.c"
    #string
    "src/string/cstr.c"
)

set(
    HEADERS
    #core
    "include/UtiC/core/event_bus.h"
    "include/UtiC/core/types.h"
    #memory
    "include/UtiC/memory/allocator.h"
    "include/UtiC/memory/allocator_counter.h"
    "include/UtiC/memory/allocator_system.h"
    "include/UtiC/memory/arena.h"
    #containers
    "include/UtiC/containers/darray.h"
    #string
    "include/UtiC/string/cstr.h"
)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options( ${PROJECT_NAME} PRIVATE /W4)
else()
    include(CheckCCompilerFlag)
    set(
        UTIC_WARNING_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wformat=2
        -Wundef
    )
    set(UTIC_ENABLED_WARNING_FLAGS)
    foreach(WARNING_FLAG IN LISTS UTIC_WARNING_FLAGS)
        string(MAKE_C_IDENTIFIER "${WARNING_FLAG}" WARNING_FLAG_ID)
        check_c_compiler_flag("${WARNING_FLAG}" "UTIC_SUPPORTS_${WARNING_FLAG_ID}")
        if(UTIC_SUPPORTS_${WARNING_FLAG_ID})
            list(APPEND UTIC_ENABLED_WARNING_FLAGS "${WARNING_FLAG}")
        endif()
    endforeach()

    if(UTIC_ENABLED_WARNING_FLAGS)
        target_compile_options( ${PROJECT_NAME} PRIVATE ${UTIC_ENABLED_WARNING_FLAGS})
    else()
        message(WARNING "UtiC: the C compiler accepted none of the requested warning flags")
    endif()
endif()
