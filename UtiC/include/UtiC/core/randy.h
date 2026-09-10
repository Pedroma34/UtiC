#pragma once

#include <UtiC/core/types.h>

/*
 * Supported:
 *     i8, i16, i32, i64, u8, u16, u32, u64,
 *     isz, usz, bool, f32, f64
 *
 * Usage:
 *     Randy rng = randy_init(randy_get_seed_time());
 *     // Use randy_init(42) instead for a repeatable sequence.
 *     u32 value = RANDY(u32, &rng);             // Full u32 range.
 *     u32 dice = RANDY_RANGE(u32, &rng, 1, 7);  // 1 through 6.
 *     i32 offset = RANDY_RANGE(i32, &rng, -10, 11); // -10 through 10.
 *     f32 chance = RANDY(f32, &rng);            // [0, 1).
 *     bool flip = RANDY(bool, &rng);            // TRUE or FALSE.
 *
 * RANDY_RANGE supports integer types only; min is inclusive, max exclusive.
 */

typedef struct Randy {
    usz state[4];
} Randy;

/* Best-effort time-derived seed; not unique or suitable for security. */
u64 randy_get_seed_time(void);
Randy randy_init(u64 seed);
void randy_jump(Randy* randy);
void randy_long_jump(Randy* randy);

#include <UtiC/core/detail/randy.inl.h>

#define RANDY(type, randy_ptr) randy_next_##type((randy_ptr))

#define RANDY_RANGE(type, randy_ptr, min_value, max_value) \
    randy_range_##type((randy_ptr), (min_value), (max_value))
