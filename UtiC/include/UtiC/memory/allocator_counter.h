/*
 * Wraps an allocator and tracks live, total, and peak allocation statistics.
 * Use the wrapped allocator for every allocation and deallocation:
 *
 * AllocatorCounter counter = allocator_counter_create(backing, NULL, NULL);
 * Allocator allocator = allocator_counter_get_allocator(&counter);
 * void* block = allocator_alloc(&allocator, 64, 8);
 * allocator_free(&allocator, block, 64, 8);
 */

#pragma once
#include "UtiC/core/types.h"
#include "UtiC/memory/allocator.h"

typedef enum AllocatorCounterEvent {
    ALLOCATOR_COUNTER_EVENT_ALLOC,
    ALLOCATOR_COUNTER_EVENT_FREE
} AllocatorCounterEvent;

typedef struct AllocatorCounterInfo {
    usz live_bytes;
    usz live_allocations;
    usz total_allocations;
    usz total_deallocations;
    usz peak_bytes;
} AllocatorCounterInfo;

typedef void(*AllocatorCounterCallback)( AllocatorCounterEvent event, const AllocatorCounterInfo* info, usz size, void* callback_user);

typedef struct AllocatorCounter {
    Allocator backing;
    AllocatorCounterCallback callback;
    void* callback_user;
    AllocatorCounterInfo info;
} AllocatorCounter;

/* callback can be NULL. */
AllocatorCounter allocator_counter_create(Allocator backing_allocator, AllocatorCounterCallback callback, void* callback_user);
Allocator allocator_counter_get_allocator(AllocatorCounter* counter);
