#if defined(__linux__) && !defined(_POSIX_C_SOURCE)
    #define _POSIX_C_SOURCE 200809L
#endif

#include <UtiC/core/randy.h>

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(__linux__)
    #include <time.h>
#else
    #error "randy_get_seed_time() supports only Windows and Linux"
#endif

#if SIZE_MAX == UINT64_MAX
    #define RANDY_WORD_BITS 64
#elif SIZE_MAX == UINT32_MAX
    #define RANDY_WORD_BITS 32
#else
    #error "Randy supports only 32-bit and 64-bit platforms"
#endif

/*
 * SplitMix64 reference algorithm by Sebastiano Vigna.
 * https://prng.di.unimi.it/splitmix64.c
 */
static u64 randy_splitmix64_next(u64* state) {
    u64 value = (*state += UINT64_C(0x9E3779B97F4A7C15));
    value = (value ^ (value >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94D049BB133111EB);
    return value ^ (value >> 31);
}

u64 randy_get_seed_time(void) {
    u64 wall_time;
    u64 fine_time;

#if defined(_WIN32)
    FILETIME file_time;
    LARGE_INTEGER counter = { 0 };

    GetSystemTimePreciseAsFileTime(&file_time);
    (void)QueryPerformanceCounter(&counter);

    wall_time =
        ((u64)file_time.dwHighDateTime << 32) |
        (u64)file_time.dwLowDateTime;
    fine_time = (u64)counter.QuadPart;
#else
    struct timespec realtime = { 0 };
    struct timespec monotonic = { 0 };

    if(clock_gettime(CLOCK_REALTIME, &realtime) != 0)
        (void)timespec_get(&realtime, TIME_UTC);
    if(clock_gettime(CLOCK_MONOTONIC, &monotonic) != 0)
        monotonic = realtime;

    wall_time =
        (u64)realtime.tv_sec * UINT64_C(1000000000) +
        (u64)realtime.tv_nsec;
    fine_time =
        (u64)monotonic.tv_sec * UINT64_C(1000000000) +
        (u64)monotonic.tv_nsec;
#endif

    const u64 rotated_fine = (fine_time << 23) | (fine_time >> 41);
    u64 combined = wall_time ^ rotated_fine;
    return randy_splitmix64_next(&combined);
}

Randy randy_init(u64 seed) {
    Randy randy = { 0 };
    u64 seed_state = seed;
#if RANDY_WORD_BITS == 64
    for(usz i = 0; i < 4; i++)
        randy.state[i] = (usz)randy_splitmix64_next(&seed_state);
#else
    const u64 first = randy_splitmix64_next(&seed_state);
    const u64 second = randy_splitmix64_next(&seed_state);
    randy.state[0] = (usz)(u32)first;
    randy.state[1] = (usz)(u32)(first >> 32);
    randy.state[2] = (usz)(u32)second;
    randy.state[3] = (usz)(u32)(second >> 32);
#endif
    return randy;
}

static void randy_apply_jump(Randy* randy, const usz jump[4]) {
    assert(randy != NULL);
    usz state[4] = { 0 };

    for(usz i = 0; i < 4; i++) {
        for(u32 bit = 0; bit < RANDY_WORD_BITS; bit++) {
            if((jump[i] & ((usz)1 << bit)) != 0) {
                state[0] ^= randy->state[0];
                state[1] ^= randy->state[1];
                state[2] ^= randy->state[2];
                state[3] ^= randy->state[3];
            }
            (void)randy_detail_next_native(randy);
        }
    }

    randy->state[0] = state[0];
    randy->state[1] = state[1];
    randy->state[2] = state[2];
    randy->state[3] = state[3];
}

void randy_jump(Randy* randy) {
#if RANDY_WORD_BITS == 64
    static const usz JUMP[4] = {
        (usz)UINT64_C(0x180EC6D33CFD0ABA),
        (usz)UINT64_C(0xD5A61266F0C9392C),
        (usz)UINT64_C(0xA9582618E03FC9AA),
        (usz)UINT64_C(0x39ABDC4529B1661C)
    };
#else
    static const usz JUMP[4] = {
        (usz)UINT32_C(0x8764000B),
        (usz)UINT32_C(0xF542D2D3),
        (usz)UINT32_C(0x6FA035C3),
        (usz)UINT32_C(0x77F2DB5B)
    };
#endif
    randy_apply_jump(randy, JUMP);
}

void randy_long_jump(Randy* randy) {
#if RANDY_WORD_BITS == 64
    static const usz LONG_JUMP[4] = {
        (usz)UINT64_C(0x76E15D3EFEFDCBBF),
        (usz)UINT64_C(0xC5004E441C522FB3),
        (usz)UINT64_C(0x77710069854EE241),
        (usz)UINT64_C(0x39109BB02ACBE635)
    };
#else
    static const usz LONG_JUMP[4] = {
        (usz)UINT32_C(0xB523952E),
        (usz)UINT32_C(0x0B6F099F),
        (usz)UINT32_C(0xCCF5A0EF),
        (usz)UINT32_C(0x1C580662)
    };
#endif
    randy_apply_jump(randy, LONG_JUMP);
}

#undef RANDY_WORD_BITS
