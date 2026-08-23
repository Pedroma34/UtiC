#if defined(__linux__) && !defined(_POSIX_C_SOURCE)
    #define _POSIX_C_SOURCE 200809L
#endif

#include <UtiC/core/randy.h>

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#define RANDY_BENCHMARK_DEFAULT_ITERATIONS UINT64_C(10000000)
#define RANDY_BENCHMARK_WARMUP_LIMIT UINT64_C(100000)
#define RANDY_BENCHMARK_SEED UINT64_C(0x6a09e667f3bcc909)

typedef struct BenchmarkResult {
    double elapsed_seconds;
    u64 checksum;
} BenchmarkResult;

static int benchmark_now(double* seconds) {
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) {
        return 0;
    }
    *seconds = (double)counter.QuadPart / (double)frequency.QuadPart;
    return 1;
#elif defined(__linux__)
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0;
    }
    *seconds = (double)now.tv_sec + (double)now.tv_nsec * 1.0e-9;
    return 1;
#else
    struct timespec now;
    if (timespec_get(&now, TIME_UTC) != TIME_UTC) {
        return 0;
    }
    *seconds = (double)now.tv_sec + (double)now.tv_nsec * 1.0e-9;
    return 1;
#endif
}

static int parse_iterations(const char* text, u64* iterations) {
    char* end = NULL;
    unsigned long long parsed;

    if (text == NULL || text[0] < '0' || text[0] > '9') {
        return 0;
    }

    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || parsed == 0ULL ||
        parsed > (unsigned long long)UINT64_MAX) {
        return 0;
    }

    *iterations = (u64)parsed;
    return 1;
}

static u64 f32_checksum_bits(f32 value) {
    u32 bits;
    memcpy(&bits, &value, sizeof(bits));
    return (u64)bits;
}

static u64 f64_checksum_bits(f64 value) {
    u64 bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static u64 warm_up(u64 iterations) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;

    for (index = 0; index < iterations; ++index) {
        checksum += (u64)RANDY(u32, &rng);
        checksum += RANDY(u64, &rng);
        checksum += f32_checksum_bits(RANDY(f32, &rng));
        checksum += f64_checksum_bits(RANDY(f64, &rng));
        checksum += (u64)RANDY_RANGE(u32, &rng, UINT32_C(37), UINT32_C(1000000007));
        checksum += RANDY_RANGE(
            u64,
            &rng,
            UINT64_C(1009),
            UINT64_C(0xd000000000000123)
        );
    }

    return checksum;
}

static int benchmark_u32(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += (u64)RANDY(u32, &rng);
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static int benchmark_u64(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += RANDY(u64, &rng);
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static int benchmark_f32(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += f32_checksum_bits(RANDY(f32, &rng));
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static int benchmark_f64(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += f64_checksum_bits(RANDY(f64, &rng));
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static int benchmark_range_u32(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += (u64)RANDY_RANGE(
            u32,
            &rng,
            UINT32_C(37),
            UINT32_C(1000000007)
        );
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static int benchmark_range_u64(u64 iterations, BenchmarkResult* result) {
    Randy rng = randy_init(RANDY_BENCHMARK_SEED);
    u64 checksum = 0;
    u64 index;
    double started;
    double finished;

    if (!benchmark_now(&started)) {
        return 0;
    }
    for (index = 0; index < iterations; ++index) {
        checksum += RANDY_RANGE(
            u64,
            &rng,
            UINT64_C(1009),
            UINT64_C(0xd000000000000123)
        );
    }
    if (!benchmark_now(&finished)) {
        return 0;
    }

    result->elapsed_seconds = finished - started;
    result->checksum = checksum;
    return result->elapsed_seconds > 0.0;
}

static void print_result(const char* name, u64 iterations, const BenchmarkResult* result) {
    const double values_per_second = (double)iterations / result->elapsed_seconds;
    const double nanoseconds_per_value = result->elapsed_seconds * 1.0e9 / (double)iterations;

    printf(
        "%-12s %12.2f M values/s  %10.3f ns/value  checksum=%016" PRIx64 "\n",
        name,
        values_per_second / 1.0e6,
        nanoseconds_per_value,
        result->checksum
    );
}

static int run_benchmark(
    const char* name,
    u64 iterations,
    int (*benchmark)(u64, BenchmarkResult*)
) {
    BenchmarkResult result;
    if (!benchmark(iterations, &result)) {
        fprintf(stderr, "Unable to time %s (timer failed or interval was too short).\n", name);
        return 0;
    }
    print_result(name, iterations, &result);
    return 1;
}

int main(int argc, char** argv) {
    u64 iterations = RANDY_BENCHMARK_DEFAULT_ITERATIONS;
    u64 warmup_iterations;
    u64 warmup_checksum;

    if (argc > 2 || (argc == 2 && !parse_iterations(argv[1], &iterations))) {
        fprintf(
            stderr,
            "Usage: %s [positive-iteration-count]\n",
            argc > 0 ? argv[0] : "UtiCRandyBenchmark"
        );
        return EXIT_FAILURE;
    }

    warmup_iterations = iterations < RANDY_BENCHMARK_WARMUP_LIMIT
        ? iterations
        : RANDY_BENCHMARK_WARMUP_LIMIT;
    warmup_checksum = warm_up(warmup_iterations);

    printf("Randy throughput benchmark\n");
    printf("iterations: %" PRIu64 " per operation\n", iterations);
    printf(
        "warm-up:    %" PRIu64 " per operation, checksum=%016" PRIx64 "\n\n",
        warmup_iterations,
        warmup_checksum
    );

    if (!run_benchmark("u32", iterations, benchmark_u32) ||
        !run_benchmark("u64", iterations, benchmark_u64) ||
        !run_benchmark("f32", iterations, benchmark_f32) ||
        !run_benchmark("f64", iterations, benchmark_f64) ||
        !run_benchmark("range u32", iterations, benchmark_range_u32) ||
        !run_benchmark("range u64", iterations, benchmark_range_u64)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
