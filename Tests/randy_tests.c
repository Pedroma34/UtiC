#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef NDEBUG
static unsigned int randy_test_assertion_failures;

static void randy_test_assert(int condition) {
    if(!condition)
        randy_test_assertion_failures++;
}

#define RANDY_DETAIL_ASSERT(condition) randy_test_assert((condition) != 0)
#endif

#include <UtiC/core/randy.h>
#include <UtiC/core/randy.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if(!(condition)) {                                                      \
            fprintf(                                                           \
                stderr,                                                         \
                "%s:%d: check failed: %s\n",                                   \
                __FILE__,                                                       \
                __LINE__,                                                       \
                #condition                                                      \
            );                                                                  \
            return FALSE;                                                       \
        }                                                                       \
    } while(0)

#define HAS_TYPE(expression, type) _Generic((expression), type: 1, default: 0)

_Static_assert(sizeof(Randy) == sizeof(usz) * 4U, "Randy contains only four native state words");
_Static_assert(HAS_TYPE(randy_get_seed_time(), u64), "randy_get_seed_time result type");

_Static_assert(HAS_TYPE(randy_next_i8((Randy*)0), i8), "randy_next_i8 result type");
_Static_assert(HAS_TYPE(randy_next_i16((Randy*)0), i16), "randy_next_i16 result type");
_Static_assert(HAS_TYPE(randy_next_i32((Randy*)0), i32), "randy_next_i32 result type");
_Static_assert(HAS_TYPE(randy_next_i64((Randy*)0), i64), "randy_next_i64 result type");
_Static_assert(HAS_TYPE(randy_next_u8((Randy*)0), u8), "randy_next_u8 result type");
_Static_assert(HAS_TYPE(randy_next_u16((Randy*)0), u16), "randy_next_u16 result type");
_Static_assert(HAS_TYPE(randy_next_u32((Randy*)0), u32), "randy_next_u32 result type");
_Static_assert(HAS_TYPE(randy_next_u64((Randy*)0), u64), "randy_next_u64 result type");
_Static_assert(HAS_TYPE(randy_next_isz((Randy*)0), isz), "randy_next_isz result type");
_Static_assert(HAS_TYPE(randy_next_usz((Randy*)0), usz), "randy_next_usz result type");
_Static_assert(HAS_TYPE(randy_next_bool((Randy*)0), bool), "randy_next_bool result type");
_Static_assert(HAS_TYPE(randy_next_f32((Randy*)0), f32), "randy_next_f32 result type");
_Static_assert(HAS_TYPE(randy_next_f64((Randy*)0), f64), "randy_next_f64 result type");

_Static_assert(HAS_TYPE(RANDY(i8, (Randy*)0), i8), "RANDY i8 result type");
_Static_assert(HAS_TYPE(RANDY(i16, (Randy*)0), i16), "RANDY i16 result type");
_Static_assert(HAS_TYPE(RANDY(i32, (Randy*)0), i32), "RANDY i32 result type");
_Static_assert(HAS_TYPE(RANDY(i64, (Randy*)0), i64), "RANDY i64 result type");
_Static_assert(HAS_TYPE(RANDY(u8, (Randy*)0), u8), "RANDY u8 result type");
_Static_assert(HAS_TYPE(RANDY(u16, (Randy*)0), u16), "RANDY u16 result type");
_Static_assert(HAS_TYPE(RANDY(u32, (Randy*)0), u32), "RANDY u32 result type");
_Static_assert(HAS_TYPE(RANDY(u64, (Randy*)0), u64), "RANDY u64 result type");
_Static_assert(HAS_TYPE(RANDY(isz, (Randy*)0), isz), "RANDY isz result type");
_Static_assert(HAS_TYPE(RANDY(usz, (Randy*)0), usz), "RANDY usz result type");
_Static_assert(HAS_TYPE(RANDY(bool, (Randy*)0), bool), "RANDY bool result type");
_Static_assert(HAS_TYPE(RANDY(f32, (Randy*)0), f32), "RANDY f32 result type");
_Static_assert(HAS_TYPE(RANDY(f64, (Randy*)0), f64), "RANDY f64 result type");
_Static_assert(
    HAS_TYPE(RANDY_RANGE(i8, (Randy*)0, (i8)0, (i8)1), i8),
    "RANDY_RANGE i8 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(i16, (Randy*)0, (i16)0, (i16)1), i16),
    "RANDY_RANGE i16 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(i32, (Randy*)0, (i32)0, (i32)1), i32),
    "RANDY_RANGE i32 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(i64, (Randy*)0, (i64)0, (i64)1), i64),
    "RANDY_RANGE i64 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(u8, (Randy*)0, (u8)0, (u8)1), u8),
    "RANDY_RANGE u8 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(u16, (Randy*)0, (u16)0, (u16)1), u16),
    "RANDY_RANGE u16 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(u32, (Randy*)0, (u32)0, (u32)1), u32),
    "RANDY_RANGE u32 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(u64, (Randy*)0, (u64)0, (u64)1), u64),
    "RANDY_RANGE u64 result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(isz, (Randy*)0, (isz)0, (isz)1), isz),
    "RANDY_RANGE isz result type"
);
_Static_assert(
    HAS_TYPE(RANDY_RANGE(usz, (Randy*)0, (usz)0, (usz)1), usz),
    "RANDY_RANGE usz result type"
);

#if SIZE_MAX == UINT64_MAX
    #define TEST_ISZ_MIN INT64_MIN
    #define TEST_ISZ_MAX INT64_MAX
#elif SIZE_MAX == UINT32_MAX
    #define TEST_ISZ_MIN INT32_MIN
    #define TEST_ISZ_MAX INT32_MAX
#else
    #error "Randy tests require a 32-bit or 64-bit size_t"
#endif

typedef struct KnownVector {
    u64 seed;
    usz initial_state[4];
    usz output[4];
} KnownVector;

static bool state_equal(const Randy* left, const Randy* right) {
    for(usz i = 0; i < 4; i++) {
        if(left->state[i] != right->state[i])
            return FALSE;
    }
    return TRUE;
}

static bool state_is_nonzero(const Randy* randy) {
    return randy->state[0] != 0 || randy->state[1] != 0 ||
           randy->state[2] != 0 || randy->state[3] != 0;
}

static usz next_native(Randy* randy) {
#if SIZE_MAX == UINT64_MAX
    return (usz)randy_next_u64(randy);
#else
    return (usz)randy_next_u32(randy);
#endif
}

static bool test_known_answer_vectors(void) {
#if SIZE_MAX == UINT64_MAX
    static const KnownVector vectors[] = {
        {
            UINT64_C(0),
            {
                (usz)UINT64_C(0xE220A8397B1DCDAF),
                (usz)UINT64_C(0x6E789E6AA1B965F4),
                (usz)UINT64_C(0x06C45D188009454F),
                (usz)UINT64_C(0xF88BB8A8724C81EC)
            },
            {
                (usz)UINT64_C(0x99EC5F36CB75F2B4),
                (usz)UINT64_C(0xBF6E1F784956452A),
                (usz)UINT64_C(0x1A5F849D4933E6E0),
                (usz)UINT64_C(0x6AA594F1262D2D2C)
            }
        },
        {
            UINT64_C(1),
            {
                (usz)UINT64_C(0x910A2DEC89025CC1),
                (usz)UINT64_C(0xBEEB8DA1658EEC67),
                (usz)UINT64_C(0xF893A2EEFB32555E),
                (usz)UINT64_C(0x71C18690EE42C90B)
            },
            {
                (usz)UINT64_C(0xB3F2AF6D0FC710C5),
                (usz)UINT64_C(0x853B559647364CEA),
                (usz)UINT64_C(0x92F89756082A4514),
                (usz)UINT64_C(0x642E1C7BC266A3A7)
            }
        },
        {
            UINT64_MAX,
            {
                (usz)UINT64_C(0xE4D971771B652C20),
                (usz)UINT64_C(0xE99FF867DBF682C9),
                (usz)UINT64_C(0x382FF84CB27281E9),
                (usz)UINT64_C(0x6D1DB36CCBA982D2)
            },
            {
                (usz)UINT64_C(0x8F5520D52A7EAD08),
                (usz)UINT64_C(0xC476A018CAA1802D),
                (usz)UINT64_C(0x81DE31C0D260469E),
                (usz)UINT64_C(0xBF658D7E065F3C2F)
            }
        }
    };
#else
    static const KnownVector vectors[] = {
        {
            UINT64_C(0),
            {
                (usz)UINT32_C(0x7B1DCDAF),
                (usz)UINT32_C(0xE220A839),
                (usz)UINT32_C(0xA1B965F4),
                (usz)UINT32_C(0x6E789E6A)
            },
            {
                (usz)UINT32_C(0xDEC9045D),
                (usz)UINT32_C(0x9A089D75),
                (usz)UINT32_C(0xAB77D362),
                (usz)UINT32_C(0xC3E16405)
            }
        },
        {
            UINT64_C(1),
            {
                (usz)UINT32_C(0x89025CC1),
                (usz)UINT32_C(0x910A2DEC),
                (usz)UINT32_C(0x658EEC67),
                (usz)UINT32_C(0xBEEB8DA1)
            },
            {
                (usz)UINT32_C(0x650941BA),
                (usz)UINT32_C(0x54D30301),
                (usz)UINT32_C(0x25D2F321),
                (usz)UINT32_C(0x3FABDCA9)
            }
        },
        {
            UINT64_MAX,
            {
                (usz)UINT32_C(0x1B652C20),
                (usz)UINT32_C(0xE4D97177),
                (usz)UINT32_C(0xDBF682C9),
                (usz)UINT32_C(0xE99FF867)
            },
            {
                (usz)UINT32_C(0x1C78F79C),
                (usz)UINT32_C(0x94A7662A),
                (usz)UINT32_C(0x211F3EA0),
                (usz)UINT32_C(0x243A6BA3)
            }
        }
    };
#endif

    const usz vector_count = sizeof(vectors) / sizeof(vectors[0]);
    for(usz vector_index = 0; vector_index < vector_count; vector_index++) {
        Randy randy = randy_init(vectors[vector_index].seed);
        CHECK(state_is_nonzero(&randy));
        for(usz word = 0; word < 4; word++)
            CHECK(randy.state[word] == vectors[vector_index].initial_state[word]);
        for(usz output = 0; output < 4; output++)
            CHECK(next_native(&randy) == vectors[vector_index].output[output]);
    }
    return TRUE;
}

static bool test_initialization_and_copy_determinism(void) {
    static const u64 seeds[] = {
        UINT64_C(0),
        UINT64_C(1),
        UINT64_C(0x0123456789ABCDEF),
        UINT64_MAX
    };

    for(usz seed_index = 0; seed_index < sizeof(seeds) / sizeof(seeds[0]); seed_index++) {
        Randy left = randy_init(seeds[seed_index]);
        Randy right = left;
        CHECK(state_is_nonzero(&left));
        CHECK(state_equal(&left, &right));

        for(usz draw = 0; draw < 128; draw++) {
            CHECK(randy_next_u64(&left) == randy_next_u64(&right));
            CHECK(randy_next_i32(&left) == randy_next_i32(&right));
            CHECK(randy_next_f32(&left) == randy_next_f32(&right));
            CHECK(randy_next_bool(&left) == randy_next_bool(&right));
        }
        CHECK(state_equal(&left, &right));
    }
    return TRUE;
}

static bool test_time_seed_smoke(void) {
    const u64 seed = randy_get_seed_time();
    Randy randy = randy_init(seed);
    CHECK(state_is_nonzero(&randy));
    (void)randy_next_u64(&randy);
    CHECK(state_is_nonzero(&randy));
    return TRUE;
}

static bool test_mixed_width_consumption(void) {
    Randy narrow = randy_init(UINT64_C(0xA55A5AA55AA55AA5));
    Randy wide = narrow;

    CHECK(randy_next_u8(&narrow) == (u8)(randy_next_u32(&wide) >> 24));
    CHECK(state_equal(&narrow, &wide));

    narrow = randy_init(UINT64_C(0x1122334455667788));
    wide = narrow;
    CHECK(randy_next_u16(&narrow) == (u16)(randy_next_u32(&wide) >> 16));
    CHECK(state_equal(&narrow, &wide));

    narrow = randy_init(UINT64_C(0xCAFEBABEDEADBEEF));
    wide = narrow;
#if SIZE_MAX == UINT64_MAX
    CHECK(randy_next_u32(&narrow) == (u32)(randy_next_u64(&wide) >> 32));
#else
    const u32 high = randy_next_u32(&wide);
    const u32 low = randy_next_u32(&wide);
    CHECK(randy_next_u64(&narrow) == (((u64)high << 32) | (u64)low));
#endif
    CHECK(state_equal(&narrow, &wide));

    narrow = randy_init(UINT64_C(0x0F0E0D0C0B0A0908));
    wide = narrow;
#if SIZE_MAX == UINT64_MAX
    CHECK(randy_next_usz(&narrow) == (usz)randy_next_u64(&wide));
#else
    CHECK(randy_next_usz(&narrow) == (usz)randy_next_u32(&wide));
#endif
    CHECK(state_equal(&narrow, &wide));
    return TRUE;
}

static i8 ordered_i8(u8 raw) {
    if(raw <= (u8)INT8_MAX)
        return (i8)(INT8_MIN + (i16)raw);
    return (i8)((u16)raw - UINT16_C(0x0080));
}

static i16 ordered_i16(u16 raw) {
    if(raw <= (u16)INT16_MAX)
        return (i16)(INT16_MIN + (i32)raw);
    return (i16)((u32)raw - UINT32_C(0x00008000));
}

static i32 ordered_i32(u32 raw) {
    if(raw <= (u32)INT32_MAX)
        return (i32)((i64)INT32_MIN + (i64)raw);
    return (i32)(raw - UINT32_C(0x80000000));
}

static i64 ordered_i64(u64 raw) {
    if(raw <= (u64)INT64_MAX)
        return INT64_MIN + (i64)raw;
    return (i64)(raw - UINT64_C(0x8000000000000000));
}

static bool test_signed_numeric_mapping(void) {
    Randy signed_randy = randy_init(UINT64_C(0x3141592653589793));
    Randy raw_randy = signed_randy;
    for(usz draw = 0; draw < 64; draw++)
        CHECK(randy_next_i8(&signed_randy) == ordered_i8(randy_next_u8(&raw_randy)));
    CHECK(state_equal(&signed_randy, &raw_randy));

    signed_randy = randy_init(UINT64_C(0x2718281828459045));
    raw_randy = signed_randy;
    for(usz draw = 0; draw < 64; draw++)
        CHECK(randy_next_i16(&signed_randy) == ordered_i16(randy_next_u16(&raw_randy)));
    CHECK(state_equal(&signed_randy, &raw_randy));

    signed_randy = randy_init(UINT64_C(0x1618033988749894));
    raw_randy = signed_randy;
    for(usz draw = 0; draw < 64; draw++)
        CHECK(randy_next_i32(&signed_randy) == ordered_i32(randy_next_u32(&raw_randy)));
    CHECK(state_equal(&signed_randy, &raw_randy));

    signed_randy = randy_init(UINT64_C(0x1414213562373095));
    raw_randy = signed_randy;
    for(usz draw = 0; draw < 64; draw++)
        CHECK(randy_next_i64(&signed_randy) == ordered_i64(randy_next_u64(&raw_randy)));
    CHECK(state_equal(&signed_randy, &raw_randy));

    signed_randy = randy_init(UINT64_C(0x1732050807568877));
    raw_randy = signed_randy;
    for(usz draw = 0; draw < 64; draw++) {
#if SIZE_MAX == UINT64_MAX
        CHECK(randy_next_isz(&signed_randy) == (isz)ordered_i64(randy_next_u64(&raw_randy)));
#else
        CHECK(randy_next_isz(&signed_randy) == (isz)ordered_i32(randy_next_u32(&raw_randy)));
#endif
    }
    CHECK(state_equal(&signed_randy, &raw_randy));
    return TRUE;
}

static bool test_boolean_and_float_conversions(void) {
    Randy bool_randy = randy_init(UINT64_C(0xB001));
    Randy raw_bool_randy = bool_randy;
    for(usz draw = 0; draw < 256; draw++) {
        const bool actual = randy_next_bool(&bool_randy);
        const bool expected = (bool)(randy_next_u32(&raw_bool_randy) >> 31);
        CHECK(actual == expected);
        CHECK(actual == FALSE || actual == TRUE);
    }
    CHECK(state_equal(&bool_randy, &raw_bool_randy));

    Randy float_randy = randy_init(UINT64_C(0xF32));
    Randy raw_randy = float_randy;
    for(usz draw = 0; draw < 128; draw++) {
        const f32 actual = randy_next_f32(&float_randy);
        const f32 expected = (f32)(randy_next_u32(&raw_randy) >> 8) / 16777216.0f;
        CHECK(actual == expected);
        CHECK(actual >= 0.0f);
        CHECK(actual < 1.0f);
    }
    CHECK(state_equal(&float_randy, &raw_randy));

    float_randy = randy_init(UINT64_C(0xF64));
    raw_randy = float_randy;
    for(usz draw = 0; draw < 128; draw++) {
        const f64 actual = randy_next_f64(&float_randy);
        const f64 expected = (f64)(randy_next_u64(&raw_randy) >> 11) /
                             9007199254740992.0;
        CHECK(actual == expected);
        CHECK(actual >= 0.0);
        CHECK(actual < 1.0);
    }
    CHECK(state_equal(&float_randy, &raw_randy));
    return TRUE;
}

static usz rng_evaluations;
static usz min_evaluations;
static usz max_evaluations;

static Randy* observed_randy(Randy* randy) {
    rng_evaluations++;
    return randy;
}

static u32 observed_min(u32 value) {
    min_evaluations++;
    return value;
}

static u32 observed_max(u32 value) {
    max_evaluations++;
    return value;
}

static bool test_macros_and_single_evaluation(void) {
    Randy actual_randy = randy_init(UINT64_C(0xA11CE));
    Randy expected_randy = actual_randy;

    rng_evaluations = 0;
    CHECK(RANDY(u32, observed_randy(&actual_randy)) == randy_next_u32(&expected_randy));
    CHECK(rng_evaluations == 1);
    CHECK(state_equal(&actual_randy, &expected_randy));

    const bool boolean_value = RANDY(bool, &actual_randy);
    CHECK(boolean_value == FALSE || boolean_value == TRUE);

    actual_randy = randy_init(UINT64_C(0xB0A1D));
    expected_randy = actual_randy;
    rng_evaluations = 0;
    min_evaluations = 0;
    max_evaluations = 0;
    const u32 expected = randy_range_u32(&expected_randy, UINT32_C(10), UINT32_C(1000));
    const u32 actual = RANDY_RANGE(
        u32,
        observed_randy(&actual_randy),
        observed_min(UINT32_C(10)),
        observed_max(UINT32_C(1000))
    );
    CHECK(actual == expected);
    CHECK(rng_evaluations == 1);
    CHECK(min_evaluations == 1);
    CHECK(max_evaluations == 1);
    CHECK(state_equal(&actual_randy, &expected_randy));
    return TRUE;
}

#define CHECK_WIDTH_ONE(type, seed, minimum, maximum)                            \
    do {                                                                        \
        Randy test_randy_ = randy_init((seed));                                 \
        const Randy before_ = test_randy_;                                      \
        const type value_ = randy_range_##type(                                 \
            &test_randy_,                                                       \
            (type)(minimum),                                                    \
            (type)(maximum)                                                     \
        );                                                                      \
        CHECK(value_ == (type)(minimum));                                       \
        CHECK(state_equal(&test_randy_, &before_));                             \
    } while(0)

#define CHECK_RANGE_BOUNDS(type, randy, minimum, maximum, iterations)            \
    do {                                                                        \
        for(usz range_index_ = 0; range_index_ < (iterations); range_index_++) { \
            const type value_ = randy_range_##type(                             \
                &(randy),                                                       \
                (type)(minimum),                                                \
                (type)(maximum)                                                 \
            );                                                                  \
            CHECK(value_ >= (type)(minimum));                                   \
            CHECK(value_ < (type)(maximum));                                    \
        }                                                                       \
    } while(0)

#ifdef NDEBUG
    #define CHECK_INVALID_RANGE(type, seed, minimum, maximum)                    \
        do {                                                                    \
            Randy test_randy_ = randy_init((seed));                             \
            const Randy before_ = test_randy_;                                  \
            const type value_ = randy_range_##type(                             \
                &test_randy_,                                                   \
                (type)(minimum),                                                \
                (type)(maximum)                                                 \
            );                                                                  \
            CHECK(value_ == (type)(minimum));                                   \
            CHECK(state_equal(&test_randy_, &before_));                         \
        } while(0)
#endif

static bool test_invalid_range_assertions(void) {
#ifndef NDEBUG
    Randy randy = randy_init(UINT64_C(0xA55E47));
    const Randy before = randy;
    randy_test_assertion_failures = 0;

    CHECK(randy_range_i8(&randy, (i8)2, (i8)1) == (i8)2);
    CHECK(randy_range_i16(&randy, (i16)2, (i16)1) == (i16)2);
    CHECK(randy_range_i32(&randy, (i32)2, (i32)1) == (i32)2);
    CHECK(randy_range_i64(&randy, (i64)2, (i64)1) == (i64)2);
    CHECK(randy_range_u8(&randy, (u8)2, (u8)1) == (u8)2);
    CHECK(randy_range_u16(&randy, (u16)2, (u16)1) == (u16)2);
    CHECK(randy_range_u32(&randy, (u32)2, (u32)1) == (u32)2);
    CHECK(randy_range_u64(&randy, (u64)2, (u64)1) == (u64)2);
    CHECK(randy_range_isz(&randy, (isz)2, (isz)1) == (isz)2);
    CHECK(randy_range_usz(&randy, (usz)2, (usz)1) == (usz)2);
    CHECK(randy_test_assertion_failures == 10U);
    CHECK(state_equal(&randy, &before));
#endif
    return TRUE;
}

static bool test_integer_range_boundaries(void) {
    CHECK_WIDTH_ONE(i8, UINT64_C(1), INT8_MAX - 1, INT8_MAX);
    CHECK_WIDTH_ONE(i16, UINT64_C(2), INT16_MAX - 1, INT16_MAX);
    CHECK_WIDTH_ONE(i32, UINT64_C(3), INT32_MAX - 1, INT32_MAX);
    CHECK_WIDTH_ONE(i64, UINT64_C(4), INT64_MAX - 1, INT64_MAX);
    CHECK_WIDTH_ONE(u8, UINT64_C(5), UINT8_MAX - 1, UINT8_MAX);
    CHECK_WIDTH_ONE(u16, UINT64_C(6), UINT16_MAX - 1, UINT16_MAX);
    CHECK_WIDTH_ONE(u32, UINT64_C(7), UINT32_MAX - 1, UINT32_MAX);
    CHECK_WIDTH_ONE(u64, UINT64_C(8), UINT64_MAX - 1, UINT64_MAX);
    CHECK_WIDTH_ONE(isz, UINT64_C(9), TEST_ISZ_MAX - 1, TEST_ISZ_MAX);
    CHECK_WIDTH_ONE(usz, UINT64_C(10), SIZE_MAX - 1, SIZE_MAX);

    Randy randy = randy_init(UINT64_C(0xD15EA5E));
    CHECK_RANGE_BOUNDS(i8, randy, INT8_MIN, INT8_MAX, 256);
    CHECK_RANGE_BOUNDS(i16, randy, INT16_MIN, INT16_MAX, 256);
    CHECK_RANGE_BOUNDS(i32, randy, INT32_MIN, INT32_MAX, 256);
    CHECK_RANGE_BOUNDS(i64, randy, INT64_MIN, INT64_MAX, 256);
    CHECK_RANGE_BOUNDS(isz, randy, TEST_ISZ_MIN, TEST_ISZ_MAX, 256);
    CHECK_RANGE_BOUNDS(u8, randy, 0, UINT8_MAX, 256);
    CHECK_RANGE_BOUNDS(u16, randy, 0, UINT16_MAX, 256);
    CHECK_RANGE_BOUNDS(u32, randy, 0, UINT32_MAX, 256);
    CHECK_RANGE_BOUNDS(u64, randy, 0, UINT64_MAX, 256);
    CHECK_RANGE_BOUNDS(usz, randy, 0, SIZE_MAX, 256);

    CHECK_RANGE_BOUNDS(i8, randy, -17, 23, 256);
    CHECK_RANGE_BOUNDS(i16, randy, -1700, 2300, 256);
    CHECK_RANGE_BOUNDS(i32, randy, -170000, 230000, 256);
    CHECK_RANGE_BOUNDS(i64, randy, INT64_C(-17000000000), INT64_C(23000000000), 256);
    CHECK_RANGE_BOUNDS(isz, randy, -17000, 23000, 256);
    CHECK_RANGE_BOUNDS(u8, randy, UINT8_MAX - 17, UINT8_MAX, 256);
    CHECK_RANGE_BOUNDS(u16, randy, UINT16_MAX - 17, UINT16_MAX, 256);
    CHECK_RANGE_BOUNDS(u32, randy, UINT32_MAX - 17, UINT32_MAX, 256);
    CHECK_RANGE_BOUNDS(u64, randy, UINT64_MAX - 17, UINT64_MAX, 256);
    CHECK_RANGE_BOUNDS(usz, randy, SIZE_MAX - 17, SIZE_MAX, 256);

#ifdef NDEBUG
    CHECK_INVALID_RANGE(i8, UINT64_C(11), 2, 1);
    CHECK_INVALID_RANGE(i16, UINT64_C(12), 2, 1);
    CHECK_INVALID_RANGE(i32, UINT64_C(13), 2, 1);
    CHECK_INVALID_RANGE(i64, UINT64_C(14), 2, 1);
    CHECK_INVALID_RANGE(u8, UINT64_C(15), 2, 1);
    CHECK_INVALID_RANGE(u16, UINT64_C(16), 2, 1);
    CHECK_INVALID_RANGE(u32, UINT64_C(17), 2, 1);
    CHECK_INVALID_RANGE(u64, UINT64_C(18), 2, 1);
    CHECK_INVALID_RANGE(isz, UINT64_C(19), 2, 1);
    CHECK_INVALID_RANGE(usz, UINT64_C(20), 2, 1);

    CHECK_INVALID_RANGE(i8, UINT64_C(21), 1, 1);
    CHECK_INVALID_RANGE(i16, UINT64_C(22), 1, 1);
    CHECK_INVALID_RANGE(i32, UINT64_C(23), 1, 1);
    CHECK_INVALID_RANGE(i64, UINT64_C(24), 1, 1);
    CHECK_INVALID_RANGE(u8, UINT64_C(25), 1, 1);
    CHECK_INVALID_RANGE(u16, UINT64_C(26), 1, 1);
    CHECK_INVALID_RANGE(u32, UINT64_C(27), 1, 1);
    CHECK_INVALID_RANGE(u64, UINT64_C(28), 1, 1);
    CHECK_INVALID_RANGE(isz, UINT64_C(29), 1, 1);
    CHECK_INVALID_RANGE(usz, UINT64_C(30), 1, 1);
#endif
    return TRUE;
}

static u32 reference_range_u32(Randy* randy, u32 minimum, u32 maximum, usz* draws) {
    const u32 span = maximum - minimum;
    const u32 threshold = (u32)(UINT32_C(0) - span) % span;
    u64 product;
    u32 low;

    *draws = 0;
    do {
        product = (u64)randy_next_u32(randy) * (u64)span;
        low = (u32)product;
        (*draws)++;
    } while(low < threshold);
    return minimum + (u32)(product >> 32);
}

static void multiply_u64(u64 left, u64 right, u64* high, u64* low) {
    const u64 left_low = (u32)left;
    const u64 left_high = left >> 32;
    const u64 right_low = (u32)right;
    const u64 right_high = right >> 32;
    const u64 product_low = left_low * right_low;
    const u64 middle = left_high * right_low + (product_low >> 32);
    u64 assembled_middle = (u32)middle;
    const u64 carry = middle >> 32;

    assembled_middle += left_low * right_high;
    *high = left_high * right_high + carry + (assembled_middle >> 32);
    *low = (assembled_middle << 32) | (u32)product_low;
}

static u64 reference_range_u64(Randy* randy, u64 minimum, u64 maximum, usz* draws) {
    const u64 span = maximum - minimum;
    const u64 threshold = (UINT64_C(0) - span) % span;
    u64 high;
    u64 low;

    *draws = 0;
    do {
        multiply_u64(randy_next_u64(randy), span, &high, &low);
        (*draws)++;
    } while(low < threshold);
    return minimum + high;
}

static u32 ordered_key_i32(i32 value) {
    if(value < 0)
        return (u32)((i64)value - (i64)INT32_MIN);
    return UINT32_C(0x80000000) + (u32)value;
}

static u64 ordered_key_i64(i64 value) {
    if(value < 0)
        return (u64)(value - INT64_MIN);
    return UINT64_C(0x8000000000000000) + (u64)value;
}

static bool test_exact_wide_integer_ranges(void) {
    usz draws = 0;
    Randy actual = randy_init(UINT64_C(0x1234));
    Randy reference = actual;
    const u32 expected_u32 = reference_range_u32(
        &reference,
        UINT32_C(0x12345678),
        UINT32_C(0xF0000000),
        &draws
    );
    CHECK(draws >= 1);
    CHECK(
        randy_range_u32(&actual, UINT32_C(0x12345678), UINT32_C(0xF0000000)) ==
        expected_u32
    );
    CHECK(state_equal(&actual, &reference));

    actual = randy_init(UINT64_C(0x5678));
    reference = actual;
    const i32 minimum_i32 = INT32_C(-170000);
    const i32 maximum_i32 = INT32_C(230000);
    const u32 expected_i32_key = reference_range_u32(
        &reference,
        ordered_key_i32(minimum_i32),
        ordered_key_i32(maximum_i32),
        &draws
    );
    CHECK(
        randy_range_i32(&actual, minimum_i32, maximum_i32) ==
        ordered_i32(expected_i32_key)
    );
    CHECK(state_equal(&actual, &reference));

    actual = randy_init(UINT64_C(0x9ABC));
    reference = actual;
    const u64 expected_u64 = reference_range_u64(
        &reference,
        UINT64_C(0x123456789ABCDEF0),
        UINT64_C(0xF000000000000000),
        &draws
    );
    CHECK(draws >= 1);
    CHECK(
        randy_range_u64(
            &actual,
            UINT64_C(0x123456789ABCDEF0),
            UINT64_C(0xF000000000000000)
        ) == expected_u64
    );
    CHECK(state_equal(&actual, &reference));

    actual = randy_init(UINT64_C(0xDEF0));
    reference = actual;
    const i64 minimum_i64 = INT64_C(-17000000000);
    const i64 maximum_i64 = INT64_C(23000000000);
    const u64 expected_i64_key = reference_range_u64(
        &reference,
        ordered_key_i64(minimum_i64),
        ordered_key_i64(maximum_i64),
        &draws
    );
    CHECK(
        randy_range_i64(&actual, minimum_i64, maximum_i64) ==
        ordered_i64(expected_i64_key)
    );
    CHECK(state_equal(&actual, &reference));
    return TRUE;
}

static bool test_range_rejection_paths(void) {
    const u32 maximum32 = UINT32_C(0x80000001);
    bool found32 = FALSE;
    for(u64 seed = 0; seed < UINT64_C(1024) && !found32; seed++) {
        Randy reference = randy_init(seed);
        usz draws = 0;
        const u32 expected = reference_range_u32(&reference, 0, maximum32, &draws);
        if(draws > 1) {
            Randy actual = randy_init(seed);
            CHECK(randy_range_u32(&actual, 0, maximum32) == expected);
            CHECK(state_equal(&actual, &reference));
            found32 = TRUE;
        }
    }
    CHECK(found32);

    const u64 maximum64 = UINT64_C(0x8000000000000001);
    bool found64 = FALSE;
    for(u64 seed = 0; seed < UINT64_C(1024) && !found64; seed++) {
        Randy reference = randy_init(seed);
        usz draws = 0;
        const u64 expected = reference_range_u64(&reference, 0, maximum64, &draws);
        if(draws > 1) {
            Randy actual = randy_init(seed);
            CHECK(randy_range_u64(&actual, 0, maximum64) == expected);
            CHECK(state_equal(&actual, &reference));
            found64 = TRUE;
        }
    }
    CHECK(found64);
    return TRUE;
}

static bool test_jump_vectors(void) {
#if SIZE_MAX == UINT64_MAX
    static const usz expected[] = {
        (usz)UINT64_C(0x376215EDC846D62C),
        (usz)UINT64_C(0x57C0611DE8350CA7),
        (usz)UINT64_C(0xBC46A3515AFEE385),
        (usz)UINT64_C(0x06C27B341ACA7B26)
    };
#else
    static const usz expected[] = {
        (usz)UINT32_C(0xD8312459),
        (usz)UINT32_C(0x14A4B54A),
        (usz)UINT32_C(0x6B145788),
        (usz)UINT32_C(0xC5E07570)
    };
#endif
    Randy left = randy_init(UINT64_C(0));
    Randy right = left;
    randy_jump(&left);
    randy_jump(&right);
    CHECK(state_equal(&left, &right));
    for(usz index = 0; index < sizeof(expected) / sizeof(expected[0]); index++)
        CHECK(next_native(&left) == expected[index]);
    return TRUE;
}

static bool test_long_jump_vectors(void) {
#if SIZE_MAX == UINT64_MAX
    static const usz expected[] = {
        (usz)UINT64_C(0xE704A522A72937EB),
        (usz)UINT64_C(0x48C8F6CC958E7583),
        (usz)UINT64_C(0x72E3AB7DB4438116),
        (usz)UINT64_C(0x8473B5E32802C8E9)
    };
#else
    static const usz expected[] = {
        (usz)UINT32_C(0x4BA6F744),
        (usz)UINT32_C(0xF062D473),
        (usz)UINT32_C(0x911889E8),
        (usz)UINT32_C(0x6012B07E)
    };
#endif
    Randy left = randy_init(UINT64_C(0));
    Randy right = left;
    randy_long_jump(&left);
    randy_long_jump(&right);
    CHECK(state_equal(&left, &right));
    for(usz index = 0; index < sizeof(expected) / sizeof(expected[0]); index++)
        CHECK(next_native(&left) == expected[index]);
    return TRUE;
}

typedef bool (*TestFunction)(void);

typedef struct TestCase {
    const char* name;
    TestFunction function;
} TestCase;

static const TestCase TEST_CASES[] = {
    { "known_answer_vectors", test_known_answer_vectors },
    { "initialization_and_copy_determinism", test_initialization_and_copy_determinism },
    { "time_seed_smoke", test_time_seed_smoke },
    { "mixed_width_consumption", test_mixed_width_consumption },
    { "signed_numeric_mapping", test_signed_numeric_mapping },
    { "boolean_and_float_conversions", test_boolean_and_float_conversions },
    { "macros_and_single_evaluation", test_macros_and_single_evaluation },
    { "integer_range_boundaries", test_integer_range_boundaries },
    { "exact_wide_integer_ranges", test_exact_wide_integer_ranges },
    { "range_rejection_paths", test_range_rejection_paths },
    { "jump_vectors", test_jump_vectors },
    { "long_jump_vectors", test_long_jump_vectors },
    { "invalid_range_assertions", test_invalid_range_assertions }
};

static int run_test(const TestCase* test_case) {
    if(!test_case->function()) {
        fprintf(stderr, "FAILED: %s\n", test_case->name);
        return 1;
    }
    fprintf(stdout, "PASSED: %s\n", test_case->name);
    return 0;
}

int main(int argc, char** argv) {
    const usz test_count = sizeof(TEST_CASES) / sizeof(TEST_CASES[0]);

    if(argc == 1) {
        for(usz index = 0; index < test_count; index++) {
            if(run_test(&TEST_CASES[index]) != 0)
                return 1;
        }
        return 0;
    }

    if(argc != 2) {
        fprintf(stderr, "usage: %s [test-name]\n", argv[0]);
        return 2;
    }

    for(usz index = 0; index < test_count; index++) {
        if(strcmp(argv[1], TEST_CASES[index].name) == 0)
            return run_test(&TEST_CASES[index]);
    }

    fprintf(stderr, "unknown test: %s\n", argv[1]);
    return 2;
}
