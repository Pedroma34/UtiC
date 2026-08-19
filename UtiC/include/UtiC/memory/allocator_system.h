/*
 * Provides an allocator backed by the operating system heap.
 * Use it as a general-purpose backing allocator:
 *
 * Allocator allocator = system_allocator();
 * void* block = allocator_alloc(&allocator, 64, 8);
 * allocator_free(&allocator, block, 64, 8);
 */

#pragma once
#include <UtiC/memory/allocator.h>

Allocator system_allocator(void);
