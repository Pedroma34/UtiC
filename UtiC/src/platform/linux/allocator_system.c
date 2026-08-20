#ifdef __linux__

#include <UtiC/memory/allocator_system.h>

#include <stdlib.h>

typedef struct LinuxAllocatorHeader {
    void* base;
} LinuxAllocatorHeader;

static bool linux_is_power_of_two(usz value) {
    return value != 0 && (value & (value - 1)) == 0;
}

static void* linux_alloc(usz size, usz alignment, void* user) {
    (void)user;

    if (size == 0)
        return NULL;

    if (alignment == 0)
        alignment = _Alignof(void*);
    if (!linux_is_power_of_two(alignment))
        return NULL;

    const usz padding = alignment - 1;
    if (padding > (usz)-1 - sizeof(LinuxAllocatorHeader))
        return NULL;

    const usz overhead = sizeof(LinuxAllocatorHeader) + padding;
    if (size > (usz)-1 - overhead)
        return NULL;

    byte* base = malloc(size + overhead);
    if (!base)
        return NULL;

    byte* unaligned = base + sizeof(LinuxAllocatorHeader);
    const usz adjustment = (usz)(-(uptr)unaligned & (uptr)padding);
    byte* block = unaligned + adjustment;

    LinuxAllocatorHeader* header = (LinuxAllocatorHeader*)(block - sizeof(LinuxAllocatorHeader));
    header->base = base;
    return block;
}

static void linux_free(void* block, usz size, usz alignment, void* user) {
    (void)size;
    (void)alignment;
    (void)user;

    if (!block)
        return;

    byte* aligned = block;
    LinuxAllocatorHeader* header = (LinuxAllocatorHeader*)(aligned - sizeof(LinuxAllocatorHeader));
    free(header->base);
}

Allocator system_allocator(void) {
    return (Allocator) {
        .alloc = linux_alloc,
        .free = linux_free,
        .user = NULL
    };
}

#endif
