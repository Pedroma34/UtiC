/*
 * Linear allocator that serves allocations from one backing block.
 * Clear it to reuse all space, then destroy it to release the block:
 *
 * Arena arena;
 * arena_create(&arena, backing, 1024);
 * Allocator allocator = arena_get_allocator(&arena);
 * u32* value = ALLOCATOR_ALLOC(u32, &allocator);
 * arena_destroy(&arena);
 */

#pragma once
#include "UtiC/core/types.h"
#include "UtiC/memory/allocator.h"

typedef struct Arena {
    Allocator backing_allocator;
    usz capacity;
    usz size;
    byte* buffer;
} Arena;

bool arena_create(Arena* arena, Allocator allocator, usz capacity);
void arena_destroy(Arena* arena);
void arena_clear(Arena* arena);
bool arena_is_empty(Arena* arena);
Allocator arena_get_allocator(Arena* arena);
