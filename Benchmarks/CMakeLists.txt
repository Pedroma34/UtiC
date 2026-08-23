add_executable(
    UtiCRandyBenchmark
    "randy_benchmark.c"
)

set_target_properties(
    UtiCRandyBenchmark
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

target_link_libraries(
    UtiCRandyBenchmark
    PRIVATE
        UtiC
)

