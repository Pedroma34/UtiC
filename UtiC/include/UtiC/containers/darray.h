#pragma once
#include "UtiC/core/types.h"
#include "UtiC/memory/allocator.h"

typedef struct DArray {
    Allocator backing_allocator;
    byte* buffer;
    usz element_size;
    usz element_alignment;
    usz size;
    usz capacity;
} DArray;

bool darray_create(DArray* darray, Allocator allocator, usz element_size, usz element_alignment, usz capacity);
void darray_destroy(DArray* darray);
void darray_clear(DArray* darray);
void* darray_push(DArray* darray);
void* darray_push_zeroed(DArray* darray);
void* darray_get_at(DArray* darray, usz index);
void darray_for_each(DArray* darray, void(*callback)(void* element));

#define DARRAY(Type) \
    struct {         \
        DArray _raw; \
        Type* _type; \
    }

#define DARRAY_CREATE(array, allocator, capacity) \
    darray_create(                                \
        &(array)->_raw,                           \
        (allocator),                              \
        sizeof(*(array)->_type),                  \
        _Alignof(__typeof__(*(array)->_type)),    \
        (capacity))

#define DARRAY_DESTROY(array) \
    darray_destroy(&(array)->_raw)

#define DARRAY_CLEAR(array) \
    darray_clear(&(array)->_raw)

#define DARRAY_PUSH(array) \
    ((__typeof__((array)->_type)) \
        darray_push(&(array)->_raw))

#define DARRAY_PUSH_ZEROED(array) \
    ((__typeof__((array)->_type)) \
        darray_push_zeroed(&(array)->_raw))

#define DARRAY_GET_AT(array, index) \
    ((__typeof__((array)->_type)) \
        darray_get_at(&(array)->_raw, (index)))

#define DARRAY_SIZE(array) \
    ((usz)(array)->_raw.size)

#define DARRAY_CAPACITY(array) \
    ((usz)(array)->_raw.capacity)
