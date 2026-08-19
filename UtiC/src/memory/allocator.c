#include <UtiC/memory/allocator.h>
#include <string.h>

static bool _is_power_of_two(usz n) {
    return n != 0 && (n & (n - 1)) == 0;
}

void* allocator_alloc(Allocator* allocator, usz size, usz alignment) {
    if(!allocator || size == 0 || alignment == 0)
        return NULL;
    if(!allocator->alloc || !_is_power_of_two(alignment))
        return NULL;
    return allocator->alloc(size, alignment, allocator->user);
}

void* allocator_alloc_array(Allocator* allocator, usz element_size, usz alignment, usz count) {
    if(element_size == 0 || count == 0 || count > (usz)-1 / element_size)
        return NULL;
    return allocator_alloc(allocator, element_size * count, alignment);
}

void* allocator_alloc_zeroed(Allocator* allocator, usz size, usz alignment) {
    void* block = allocator_alloc(allocator, size, alignment);
    if(block)
        memset(block, 0, size);
    return block;
}

void* allocator_alloc_array_zeroed(Allocator* allocator, usz element_size, usz alignment, usz count) {
    void* block = allocator_alloc_array(allocator, element_size, alignment, count);
    if(block)
        memset(block, 0, element_size * count);
    return block;
}

void allocator_free(Allocator* allocator, void* block, usz size, usz alignment) {
    if(!allocator || size == 0 || alignment == 0)
        return;
    if(!allocator->free || !_is_power_of_two(alignment))
        return;
    allocator->free(block, size, alignment, allocator->user);
}

void allocator_free_array(Allocator* allocator, void* block, usz element_size, usz alignment, usz count) {
    if(element_size == 0 || count == 0 || count > (usz)-1 / element_size)
        return;
    allocator_free(allocator, block, element_size * count, alignment);
}
