#include "UtiC/containers/darray.h"
#include <string.h>

static bool darray_check(DArray* darray) {
    if(!darray || !darray->buffer || darray->element_size == 0 || darray->element_alignment == 0)
        return FALSE;
    return TRUE;
}

static bool darray_resize(DArray* darray, usz new_capacity) {
    if(!darray || !darray->buffer || darray->element_size == 0 || new_capacity == 0)
        return FALSE;
    if(new_capacity < darray->size || new_capacity > (usz)-1 / darray->element_size)
        return FALSE;
    if(new_capacity == darray->capacity)
        return TRUE;

    byte* new_buffer = allocator_alloc_array(
        &darray->backing_allocator,
        darray->element_size,
        darray->element_alignment,
        new_capacity
    );
    if(!new_buffer)
        return FALSE;

    if(darray->size > 0)
        memcpy(new_buffer, darray->buffer, darray->size * darray->element_size);

    allocator_free_array(
        &darray->backing_allocator,
        darray->buffer,
        darray->element_size,
        darray->element_alignment,
        darray->capacity
    );
    darray->buffer   = new_buffer;
    darray->capacity = new_capacity;
    return TRUE;
}

bool darray_create(DArray* darray, Allocator allocator, usz element_size, usz element_alignment, usz capacity) {
    if(!darray || element_size == 0 || element_alignment == 0 || !allocator.alloc || !allocator.free)
        return FALSE;
    if(element_size % element_alignment != 0)
        return FALSE;
    const usz actual_capacity = (capacity == 0) ? 1 : capacity;
    darray->backing_allocator = allocator;
    darray->element_size      = element_size;
    darray->element_alignment = element_alignment;
    darray->size              = 0;
    darray->capacity          = actual_capacity;
    darray->buffer            = allocator_alloc_array(&allocator, element_size, element_alignment, actual_capacity);
    if(!darray->buffer)
        return FALSE;
    return TRUE;
}

void darray_destroy(DArray *darray) {
    if(!darray)
        return;
    if(darray->buffer)
        allocator_free_array(&darray->backing_allocator, darray->buffer, darray->element_size, darray->element_alignment, darray->capacity);
    *darray = (DArray) { 0 };
}

void darray_clear(DArray* darray) {
    if(!darray)
        return;
    darray->size = 0;
}

void* darray_push(DArray* darray) {
    if(!darray || !darray->buffer || darray->element_size == 0 || darray->element_alignment == 0)
        return NULL;
    if(!darray->backing_allocator.alloc || !darray->backing_allocator.free || darray->capacity == 0)
        return NULL;
    if(darray->size > darray->capacity)
        return NULL;
    const usz max_capacity = (usz)-1 / darray->element_size;
    if(darray->capacity > max_capacity)
        return NULL;

    if(darray->size == darray->capacity) {
        if(darray->capacity == max_capacity)
            return NULL;
        const usz new_capacity = (darray->capacity > max_capacity / 2) ? max_capacity : darray->capacity * 2;
        if(!darray_resize(darray, new_capacity))
            return NULL;
    }

    void* new_element = darray->buffer + darray->size * darray->element_size;
    darray->size += 1;
    return new_element;
}

void *darray_push_zeroed(DArray *darray) {
    void* element = darray_push(darray);
    if(!element)
        return NULL;
    for(usz i = 0; i < darray->element_size; i++)
        ((byte*)element)[i] = 0;
    return element;
}

void* darray_get_at(DArray* darray, usz index) {
    if(!darray_check(darray))
        return NULL;
    if(darray->size > darray->capacity || index >= darray->size)
        return NULL;
    if(darray->capacity > (usz)-1 / darray->element_size)
        return NULL;

    return darray->buffer + index * darray->element_size;
}

void darray_for_each(DArray* darray, void (*callback)(void* element)) {
    if(!darray_check(darray))
        return;
    for(usz i = 0; i < darray->size; i++) {
        byte* element = &darray->buffer[i * darray->element_size];
        callback((void*)element);
    }
}
