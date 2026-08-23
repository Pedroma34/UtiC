#pragma once

#include <assert.h>
#include <float.h>
#include <limits.h>

#ifndef RANDY_DETAIL_ASSERT
    #define RANDY_DETAIL_ASSERT(condition) assert(condition)
#endif

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    #include <intrin.h>
#endif

_Static_assert(CHAR_BIT == 8, "Randy requires 8-bit bytes");
_Static_assert(sizeof(u32) * CHAR_BIT == 32, "Randy requires a 32-bit u32");
_Static_assert(sizeof(u64) * CHAR_BIT == 64, "Randy requires a 64-bit u64");
_Static_assert(sizeof(isz) == sizeof(usz), "Randy requires isz and usz to have equal widths");
_Static_assert(sizeof(f32) * CHAR_BIT == 32 && FLT_RADIX == 2 &&
               FLT_MANT_DIG == 24 && FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128,
               "Randy requires IEEE-style binary32 f32");
_Static_assert(sizeof(f64) * CHAR_BIT == 64 && FLT_RADIX == 2 &&
               DBL_MANT_DIG == 53 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024,
               "Randy requires IEEE-style binary64 f64");

#if SIZE_MAX != UINT32_MAX && SIZE_MAX != UINT64_MAX
    #error "Randy supports only 32-bit and 64-bit platforms"
#endif

#if SIZE_MAX == UINT64_MAX
_Static_assert(PTRDIFF_MIN == INT64_MIN && PTRDIFF_MAX == INT64_MAX,
               "Randy requires a 64-bit two's-complement isz");
#else
_Static_assert(PTRDIFF_MIN == INT32_MIN && PTRDIFF_MAX == INT32_MAX,
               "Randy requires a 32-bit two's-complement isz");
#endif

/*
 * https://prng.di.unimi.it/xoshiro128starstar.c
 * https://prng.di.unimi.it/xoshiro256starstar.c
 */

static inline u32 randy_detail_rotate_left_u32(u32 value, u32 distance) {
    return (u32)((value << distance) | (value >> (UINT32_C(32) - distance)));
}

static inline u64 randy_detail_rotate_left_u64(u64 value, u32 distance) {
    return (u64)((value << distance) | (value >> (UINT32_C(64) - distance)));
}

#if SIZE_MAX == UINT64_MAX

static inline u64 randy_detail_next_native(Randy* randy) {
    RANDY_DETAIL_ASSERT(randy != NULL);
    RANDY_DETAIL_ASSERT((randy->state[0] | randy->state[1] | randy->state[2] | randy->state[3]) != 0);

    usz state0 = randy->state[0];
    usz state1 = randy->state[1];
    usz state2 = randy->state[2];
    usz state3 = randy->state[3];
    const u64 result = randy_detail_rotate_left_u64((u64)state1 * UINT64_C(5), 7) * UINT64_C(9);
    const usz shifted = state1 << 17;

    state2 ^= state0;
    state3 ^= state1;
    state1 ^= state2;
    state0 ^= state3;
    state2 ^= shifted;
    state3 = (usz)randy_detail_rotate_left_u64((u64)state3, 45);

    randy->state[0] = state0;
    randy->state[1] = state1;
    randy->state[2] = state2;
    randy->state[3] = state3;
    return result;
}

#else

static inline u32 randy_detail_next_native(Randy* randy) {
    RANDY_DETAIL_ASSERT(randy != NULL);
    RANDY_DETAIL_ASSERT((randy->state[0] | randy->state[1] | randy->state[2] | randy->state[3]) != 0);

    usz state0 = randy->state[0];
    usz state1 = randy->state[1];
    usz state2 = randy->state[2];
    usz state3 = randy->state[3];
    const u32 result = randy_detail_rotate_left_u32((u32)state1 * UINT32_C(5), 7) * UINT32_C(9);
    const usz shifted = state1 << 9;

    state2 ^= state0;
    state3 ^= state1;
    state1 ^= state2;
    state0 ^= state3;
    state2 ^= shifted;
    state3 = (usz)randy_detail_rotate_left_u32((u32)state3, 11);

    randy->state[0] = state0;
    randy->state[1] = state1;
    randy->state[2] = state2;
    randy->state[3] = state3;
    return result;
}

#endif

static inline u32 randy_next_u32(Randy* randy) {
#if SIZE_MAX == UINT64_MAX
    return (u32)(randy_detail_next_native(randy) >> 32);
#else
    return randy_detail_next_native(randy);
#endif
}

static inline u64 randy_next_u64(Randy* randy) {
#if SIZE_MAX == UINT64_MAX
    return randy_detail_next_native(randy);
#else
    const u64 high = (u64)randy_detail_next_native(randy) << 32;
    return high | (u64)randy_detail_next_native(randy);
#endif
}

static inline u16 randy_next_u16(Randy* randy) {
    return (u16)(randy_next_u32(randy) >> 16);
}

static inline u8 randy_next_u8(Randy* randy) {
    return (u8)(randy_next_u32(randy) >> 24);
}

static inline usz randy_next_usz(Randy* randy) {
#if SIZE_MAX == UINT64_MAX
    return (usz)randy_next_u64(randy);
#else
    return (usz)randy_next_u32(randy);
#endif
}

static inline i8 randy_detail_decode_i8(u8 value) {
    if(value <= (u8)INT8_MAX)
        return (i8)((i16)INT8_MIN + (i16)value);
    return (i8)((u16)value - ((u16)INT8_MAX + UINT16_C(1)));
}

static inline i16 randy_detail_decode_i16(u16 value) {
    if(value <= (u16)INT16_MAX)
        return (i16)((i32)INT16_MIN + (i32)value);
    return (i16)((u32)value - ((u32)INT16_MAX + UINT32_C(1)));
}

static inline i32 randy_detail_decode_i32(u32 value) {
    if(value <= (u32)INT32_MAX)
        return (i32)((i64)INT32_MIN + (i64)value);
    return (i32)(value - ((u32)INT32_MAX + UINT32_C(1)));
}

static inline i64 randy_detail_decode_i64(u64 value) {
    if(value <= (u64)INT64_MAX)
        return INT64_MIN + (i64)value;
    return (i64)(value - ((u64)INT64_MAX + UINT64_C(1)));
}

static inline u8 randy_detail_encode_i8(i8 value) {
    if(value < 0)
        return (u8)((i16)value - (i16)INT8_MIN);
    return (u8)(((u16)INT8_MAX + UINT16_C(1)) + (u16)value);
}

static inline u16 randy_detail_encode_i16(i16 value) {
    if(value < 0)
        return (u16)((i32)value - (i32)INT16_MIN);
    return (u16)(((u32)INT16_MAX + UINT32_C(1)) + (u32)value);
}

static inline u32 randy_detail_encode_i32(i32 value) {
    if(value < 0)
        return (u32)((i64)value - (i64)INT32_MIN);
    return ((u32)INT32_MAX + UINT32_C(1)) + (u32)value;
}

static inline u64 randy_detail_encode_i64(i64 value) {
    if(value < 0)
        return (u64)(value - INT64_MIN);
    return ((u64)INT64_MAX + UINT64_C(1)) + (u64)value;
}

static inline i8 randy_next_i8(Randy* randy) {
    return randy_detail_decode_i8(randy_next_u8(randy));
}

static inline i16 randy_next_i16(Randy* randy) {
    return randy_detail_decode_i16(randy_next_u16(randy));
}

static inline i32 randy_next_i32(Randy* randy) {
    return randy_detail_decode_i32(randy_next_u32(randy));
}

static inline i64 randy_next_i64(Randy* randy) {
    return randy_detail_decode_i64(randy_next_u64(randy));
}

static inline isz randy_next_isz(Randy* randy) {
#if SIZE_MAX == UINT64_MAX
    return (isz)randy_next_i64(randy);
#else
    return (isz)randy_next_i32(randy);
#endif
}

static inline bool randy_next_bool(Randy* randy) {
    return (bool)(randy_next_u32(randy) >> 31);
}

static inline f32 randy_next_f32(Randy* randy) {
    return (f32)(randy_next_u32(randy) >> 8) * (f32)(1.0 / 16777216.0);
}

static inline f64 randy_next_f64(Randy* randy) {
    return (f64)(randy_next_u64(randy) >> 11) * (1.0 / 9007199254740992.0);
}

static inline u64 randy_detail_multiply_high_u64(u64 left, u64 right, u64* low) {
#if defined(_MSC_VER) && defined(_M_X64)
    u64 high_part = 0;
    const u64 low_part = (u64)_umul128(left, right, &high_part);
    *low = low_part;
    return high_part;
#elif defined(_MSC_VER) && defined(_M_ARM64)
    *low = left * right;
    return (u64)__umulh(left, right);
#elif defined(__SIZEOF_INT128__)
    const __uint128_t product = (__uint128_t)left * (__uint128_t)right;
    *low = (u64)product;
    return (u64)(product >> 64);
#else
    const u64 mask = UINT64_C(0xFFFFFFFF);
    const u64 left_low = left & mask;
    const u64 left_high = left >> 32;
    const u64 right_low = right & mask;
    const u64 right_high = right >> 32;
    const u64 product_low = left_low * right_low;
    const u64 carry_product = left_high * right_low + (product_low >> 32);
    u64 middle = carry_product & mask;
    const u64 high_carry = carry_product >> 32;

    middle += left_low * right_high;
    *low = (middle << 32) | (product_low & mask);
    return left_high * right_high + high_carry + (middle >> 32);
#endif
}

/*
 * Unbiased range reduction follows Daniel Lemire's multiply-high method.
 * https://arxiv.org/abs/1805.10941
 */

static inline u32 randy_detail_bounded_u32(Randy* randy, u32 bound) {
    u32 value = randy_next_u32(randy);
    u64 product = (u64)value * (u64)bound;
    u32 low = (u32)product;

    if(low < bound) {
        const u32 threshold = (UINT32_C(0) - bound) % bound;
        while(low < threshold) {
            value = randy_next_u32(randy);
            product = (u64)value * (u64)bound;
            low = (u32)product;
        }
    }
    return (u32)(product >> 32);
}

#if defined(_MSC_VER)
    #define RANDY_DETAIL_BOUNDED_U64_INLINE __forceinline
#else
    #define RANDY_DETAIL_BOUNDED_U64_INLINE inline
#endif

static RANDY_DETAIL_BOUNDED_U64_INLINE u64 randy_detail_bounded_u64(Randy* randy, u64 bound) {
    u64 low = 0;
    u64 value = randy_next_u64(randy);
    u64 high = randy_detail_multiply_high_u64(value, bound, &low);

    if(low < bound) {
        const u64 threshold = (UINT64_C(0) - bound) % bound;
        while(low < threshold) {
            value = randy_next_u64(randy);
            high = randy_detail_multiply_high_u64(value, bound, &low);
        }
    }
    return high;
}

#undef RANDY_DETAIL_BOUNDED_U64_INLINE

static inline u8 randy_range_u8(Randy* randy, u8 min_value, u8 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u32 width = (u32)max_value - (u32)min_value;
    if(width == UINT32_C(1))
        return min_value;
    return (u8)((u32)min_value + randy_detail_bounded_u32(randy, width));
}

static inline u16 randy_range_u16(Randy* randy, u16 min_value, u16 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u32 width = (u32)max_value - (u32)min_value;
    if(width == UINT32_C(1))
        return min_value;
    return (u16)((u32)min_value + randy_detail_bounded_u32(randy, width));
}

static inline u32 randy_range_u32(Randy* randy, u32 min_value, u32 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u32 width = max_value - min_value;
    if(width == UINT32_C(1))
        return min_value;
    return min_value + randy_detail_bounded_u32(randy, width);
}

static inline u64 randy_range_u64(Randy* randy, u64 min_value, u64 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u64 width = max_value - min_value;
    if(width == UINT64_C(1))
        return min_value;
    return min_value + randy_detail_bounded_u64(randy, width);
}

static inline i8 randy_range_i8(Randy* randy, i8 min_value, i8 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u8 min_key = randy_detail_encode_i8(min_value);
    const u32 width = (u32)randy_detail_encode_i8(max_value) - (u32)min_key;
    if(width == UINT32_C(1))
        return min_value;
    return randy_detail_decode_i8((u8)((u32)min_key + randy_detail_bounded_u32(randy, width)));
}

static inline i16 randy_range_i16(Randy* randy, i16 min_value, i16 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u16 min_key = randy_detail_encode_i16(min_value);
    const u32 width = (u32)randy_detail_encode_i16(max_value) - (u32)min_key;
    if(width == UINT32_C(1))
        return min_value;
    return randy_detail_decode_i16((u16)((u32)min_key + randy_detail_bounded_u32(randy, width)));
}

static inline i32 randy_range_i32(Randy* randy, i32 min_value, i32 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u32 min_key = randy_detail_encode_i32(min_value);
    const u32 width = randy_detail_encode_i32(max_value) - min_key;
    if(width == UINT32_C(1))
        return min_value;
    return randy_detail_decode_i32(min_key + randy_detail_bounded_u32(randy, width));
}

static inline i64 randy_range_i64(Randy* randy, i64 min_value, i64 max_value) {
    if(min_value >= max_value) {
        RANDY_DETAIL_ASSERT(min_value < max_value);
        return min_value;
    }
    const u64 min_key = randy_detail_encode_i64(min_value);
    const u64 width = randy_detail_encode_i64(max_value) - min_key;
    if(width == UINT64_C(1))
        return min_value;
    return randy_detail_decode_i64(min_key + randy_detail_bounded_u64(randy, width));
}

static inline usz randy_range_usz(Randy* randy, usz min_value, usz max_value) {
#if SIZE_MAX == UINT64_MAX
    return (usz)randy_range_u64(randy, (u64)min_value, (u64)max_value);
#else
    return (usz)randy_range_u32(randy, (u32)min_value, (u32)max_value);
#endif
}

static inline isz randy_range_isz(Randy* randy, isz min_value, isz max_value) {
#if SIZE_MAX == UINT64_MAX
    return (isz)randy_range_i64(randy, (i64)min_value, (i64)max_value);
#else
    return (isz)randy_range_i32(randy, (i32)min_value, (i32)max_value);
#endif
}

#undef RANDY_DETAIL_ASSERT
