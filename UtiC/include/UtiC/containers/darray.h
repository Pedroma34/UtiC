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

#define DARRAY_CREATE(Type, darray, allocator, capacity) \
    darray_create((darray), (allocator), sizeof(Type), _Alignof(Type), (capacity))
#define DARRAY_DESTROY(darray) \
    darray_destroy((darray))
#define DARRAY_PUSH(Type, darray) \
    ((Type*)darray_push((darray)))
#define DARRAY_PUSH_ZEROED(Type, darray) \
    ((Type*)darray_push_zeroed((darray)))
#define DARRAY_GET_AT(Type, darray, index) \
    ((Type*)darray_get_at((darray), (index)))