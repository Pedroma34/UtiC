#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef NULL
    #define NULL (void*)0
#endif
#ifndef bool
    #define bool uint8_t
#endif
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

typedef uintptr_t uptr;
typedef size_t    usz;
typedef ptrdiff_t isz;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uint8_t   byte;
typedef double max_align; /* Most aligned type according to std::max_align_t in cstddef */

#define KB(x) ((x) * 1024ULL)
#define MB(x) (KB(x) * 1024ULL)
#define GB(x) (MB(x) * 1024ULL)
