#include "UtiC/memory/arena.h"

static uptr _align_forward(uptr ptr, usz alignment) {
    uptr mask = alignment - 1;
    return (ptr + mask) & ~mask; /* Moves the pointer and then clear the lower bits */
}

static void* _arena_alloc(usz size, usz alignment, void* user) {
    if(size == 0 || alignment == 0 || !user)
        return NULL;
    Arena* arena = (Arena*)user;
    if(!arena->buffer || arena->size > arena->capacity)
        return NULL;

    const uptr current_pos = (uptr)arena->buffer + arena->size;
    const uptr aligned = _align_forward(current_pos, alignment);
    const usz  padding = (usz)(aligned - current_pos);

    if(padding > arena->capacity - arena->size)
        return NULL;

    if (size > arena->capacity - arena->size - padding)
        return NULL;

    arena->size += padding + size;
    return (void*)aligned;
}

static void _arena_free(void* block, usz size, usz alignment, void* user) {
    (void)block;
    (void)size;
    (void)alignment;
    (void)user;
}

bool arena_create(Arena* arena, Allocator allocator, usz capacity) {
    if(!arena)
        return FALSE;
    arena->backing_allocator = allocator;
    arena->capacity = capacity;
    arena->size     = 0;
    arena->buffer   = allocator_alloc(&allocator, capacity, _Alignof(byte));
    return (arena->buffer ? TRUE : FALSE);
}

void arena_destroy(Arena* arena) {
    if(!arena)
        return;
    if(arena->buffer)
        allocator_free(&arena->backing_allocator, arena->buffer, arena->capacity, _Alignof(byte));
    *arena = (Arena) { 0 };
}

void arena_clear(Arena* arena) {
    if(!arena)
        return;
    arena->size = 0;
}

bool arena_is_empty(Arena* arena) {
    if(!arena)
        return TRUE;
    return arena->size == 0;
}

Allocator arena_get_allocator(Arena *arena) {
    if(!arena || !arena->buffer)
        return (Allocator) { 0 };
    return (Allocator) {
        .alloc = _arena_alloc,
        .free  = _arena_free,
        .user  = arena
    };
}
