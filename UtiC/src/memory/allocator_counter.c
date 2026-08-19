#include "UtiC/memory/allocator_counter.h"

static void* counter_alloc(usz size, usz alignment, void* user) {
    if(size == 0 || alignment == 0 || !user)
        return NULL;

    AllocatorCounter* counter  = (AllocatorCounter*)(user);
    AllocatorCounterInfo* info = &counter->info;
    if(!counter->backing.alloc)
        return NULL;

    void* new_data = counter->backing.alloc(size, alignment, counter->backing.user);
    if(!new_data)
        return NULL;

    info->live_allocations  += 1;
    info->live_bytes        += size;
    info->total_allocations += 1;
    if(info->peak_bytes < info->live_bytes)
        info->peak_bytes = info->live_bytes;
    if(counter->callback)
        counter->callback(ALLOCATOR_COUNTER_EVENT_ALLOC, info, size, counter->callback_user);
    return new_data;
}

static void counter_free(void* block, usz size, usz alignment, void* user) {
    if(!block || size == 0 || alignment == 0 || !user)
        return;

    AllocatorCounter* counter  = (AllocatorCounter*)(user);
    AllocatorCounterInfo* info = &counter->info;
    if(!counter->backing.free)
        return;

    counter->backing.free(block, size, alignment, counter->backing.user);
    info->live_allocations     -= 1;
    info->live_bytes           -= size;
    info->total_deallocations += 1;
    if(counter->callback)
        counter->callback( ALLOCATOR_COUNTER_EVENT_FREE, info, size, counter->callback_user);
}

AllocatorCounter allocator_counter_create(Allocator backing_allocator, AllocatorCounterCallback callback, void* callback_user) {
    return (AllocatorCounter) {
        .backing       = backing_allocator,
        .callback      = callback,
        .callback_user = callback_user,
        .info          = (AllocatorCounterInfo) { 0 }
    };
}

Allocator allocator_counter_get_allocator(AllocatorCounter* counter) {
    if(!counter)
        return (Allocator) { 0 };
    return (Allocator) {
        .alloc = counter->backing.alloc ? counter_alloc : NULL,
        .free  = counter->backing.free  ? counter_free : NULL,
        .user  = counter
    };
}
