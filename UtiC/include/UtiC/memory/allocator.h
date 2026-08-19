/*
 * Common allocation interface used by UtiC memory utilities.
 * Allocate and free a block with the same size and alignment:
 *
 * void* block = allocator_alloc(&allocator, 64, 8);
 * allocator_free(&allocator, block, 64, 8);
 */

#pragma once
#include <UtiC/core/types.h>

typedef void*(*alloc_fn)(usz size, usz alignment, void* user);
typedef void(*free_fn)(void* block, usz size, usz alignment, void* user);

typedef struct Allocator {
    alloc_fn alloc;
    free_fn  free;
    void*    user; 
} Allocator;

void* allocator_alloc(Allocator* allocator, usz size, usz alignment);
void* allocator_alloc_array(Allocator* allocator, usz element_size, usz alignment, usz count);
void* allocator_alloc_zeroed(Allocator* allocator, usz size, usz alignment);
void* allocator_alloc_array_zeroed(Allocator* allocator, usz element_size, usz alignment, usz count);
void  allocator_free(Allocator* allocator, void* block, usz size, usz alignment);
void  allocator_free_array(Allocator* allocator, void* block, usz element_size, usz alignment, usz count);

#define ALLOCATOR_ALLOC(Type, allocator) \
    allocator_alloc((allocator), sizeof(Type), _Alignof(Type))
#define ALLOCATOR_ALLOC_ARRAY(Type, allocator, count) \
    allocator_alloc_array((allocator), sizeof(Type), _Alignof(Type), (count))
#define ALLOCATOR_ALLOC_ZEROED(Type, allocator) \
    allocator_alloc_zeroed((allocator), sizeof(Type), _Alignof(Type))
#define ALLOCATOR_ALLOC_ARRAY_ZEROED(Type, allocator, count) \
    allocator_alloc_array_zeroed((allocator), sizeof(Type), _Alignof(Type), (count))
#define ALLOCATOR_FREE(Type, allocator, block) \
    allocator_free((allocator), (block), sizeof(Type), _Alignof(Type))
#define ALLOCATOR_FREE_ARRAY(Type, allocator, block, count) \
    allocator_free_array((allocator), (block), sizeof(Type), _Alignof(Type), (count))
