#include <string.h> /* memcpy */
#include "UtiC/core/event_bus.h"
#include "UtiC/memory/arena.h"
#include "UtiC/containers/darray.h"

typedef struct EventListener {
    EventCallback callback;
    EventContext context;
} EventListener;

typedef struct Event {
    EventListener listeners[EVENT_BUS_MAX_LISTENERS_PER_TYPE];
    usz listeners_count;
} Event;

#define EVENT_ARENA_SIZE KB(56)
typedef struct EventBusImpl {
    Event* registry;
    DArray queue;
    Arena payload_arena;
    bool processing;
    bool destroy_requested;
} EventBusImpl;

static void event_bus_destroy_now(EventBus* bus) {
    if(!bus || !bus->impl)
        return;

    if(bus->impl->registry) {
        ALLOCATOR_FREE_ARRAY(Event, &bus->backing_allocator, bus->impl->registry, EVENT_BUS_MAX_EVENT_TYPES);
        bus->impl->registry = NULL;
    }
    arena_destroy(&bus->impl->payload_arena);
    DARRAY_DESTROY(&bus->impl->queue);
    ALLOCATOR_FREE(EventBusImpl, &bus->backing_allocator, bus->impl);
    bus->impl = NULL;
    bus->backing_allocator = (Allocator) { 0 };
    *bus = (EventBus) { 0 };
}

bool event_bus_create(EventBus* bus, Allocator backing_allocator) {
    if(!bus)
        return FALSE;

    *bus = (EventBus) { 0 };
    if(!backing_allocator.alloc || !backing_allocator.free)
        return FALSE;

    bus->impl = ALLOCATOR_ALLOC_ZEROED(EventBusImpl, &backing_allocator);
    if(!bus->impl)
        return FALSE;
    bus->backing_allocator = backing_allocator;

    EventBusImpl* evnt_bus = bus->impl;
    evnt_bus->registry = ALLOCATOR_ALLOC_ARRAY_ZEROED(Event, &backing_allocator, EVENT_BUS_MAX_EVENT_TYPES);
    if(!evnt_bus->registry) {
        event_bus_destroy_now(bus);
        return FALSE;
    }
    if(!DARRAY_CREATE(EventListener, &evnt_bus->queue, backing_allocator, EVENT_BUS_MAX_QUEUED_DELIVERIES)) {
        event_bus_destroy_now(bus);
        return FALSE;
    }

    if(!arena_create(&evnt_bus->payload_arena, backing_allocator, EVENT_ARENA_SIZE)) {
        event_bus_destroy_now(bus);
        return FALSE;
    }

    return TRUE;
}

void event_bus_destroy(EventBus *bus) {
    if(!bus || !bus->impl)
        return;
    if(bus->impl->processing) {
        bus->impl->destroy_requested = TRUE;
        return;
    }
    event_bus_destroy_now(bus);
}

bool event_bus_subscribe(EventBus* bus, const EventSubscriber* subscriber) {
    if(!bus || !bus->impl || bus->impl->destroy_requested || !subscriber || !subscriber->callback)
        return FALSE;
    if(subscriber->code >= EVENT_BUS_MAX_EVENT_TYPES)
        return FALSE;

    EventBusImpl* event_bus = bus->impl;
    Event* event = &event_bus->registry[(u64)subscriber->code];
    if(event->listeners_count >= EVENT_BUS_MAX_LISTENERS_PER_TYPE)
        return FALSE;

    /* Looking for duplicates */
    for(usz i = 0; i < event->listeners_count; i++) {
        if(event->listeners[i].context.subscriber == subscriber->subscriber &&
           event->listeners[i].callback == subscriber->callback)
            return FALSE;
    }

    EventListener* new_listener      = &event->listeners[event->listeners_count];
    new_listener->callback           = subscriber->callback;
    new_listener->context.subscriber = subscriber->subscriber;
    new_listener->context.code       = subscriber->code;
    event->listeners_count++;
    return TRUE;
}

bool event_bus_publish(EventBus *bus, const EventContext *context) {
    if(!bus || !bus->impl || bus->impl->destroy_requested || !context ||
       !context->payload || context->payload_size == 0 || context->payload_alignment == 0)
        return FALSE;
    /* Power of two */
    if((context->payload_alignment & (context->payload_alignment - 1)) != 0)
        return FALSE;
    if(context->code >= EVENT_BUS_MAX_EVENT_TYPES)
        return FALSE;

    EventBusImpl* event_bus = bus->impl;
    Event* event = &event_bus->registry[(u64)context->code];
    if(event->listeners_count == 0)
        return FALSE;
    if(event_bus->queue.size > EVENT_BUS_MAX_QUEUED_DELIVERIES || event->listeners_count > EVENT_BUS_MAX_QUEUED_DELIVERIES - event_bus->queue.size)
        return FALSE;

    const usz queue_checkpoint = event_bus->queue.size;
    const usz arena_checkpoint = event_bus->payload_arena.size;
    EventListener staged_listeners[EVENT_BUS_MAX_LISTENERS_PER_TYPE];
    Allocator arena_allocator = arena_get_allocator(&event_bus->payload_arena);
    for(usz i = 0; i < event->listeners_count; i++) {
        EventListener* listener = &event->listeners[i];
        staged_listeners[i] = *listener;
        staged_listeners[i].context.publisher = context->publisher;
        staged_listeners[i].context.payload = allocator_alloc(&arena_allocator, context->payload_size, context->payload_alignment);
        if(!staged_listeners[i].context.payload) {
            event_bus->payload_arena.size = arena_checkpoint;
            return FALSE;
        }
        staged_listeners[i].context.payload_size      = context->payload_size;
        staged_listeners[i].context.payload_alignment = context->payload_alignment;
        memcpy(staged_listeners[i].context.payload, context->payload, context->payload_size);
    }

    for(usz i = 0; i < event->listeners_count; i++) {
        EventListener* listener_queue = darray_push(&event_bus->queue);
        if(!listener_queue) {
            event_bus->queue.size = queue_checkpoint;
            event_bus->payload_arena.size = arena_checkpoint;
            return FALSE;
        }
        *listener_queue = staged_listeners[i];
    }

    return TRUE;
}


bool event_bus_process(EventBus* bus) {
    if(!bus || !bus->impl || bus->impl->processing || bus->impl->destroy_requested)
        return FALSE;
    EventBusImpl* event_bus = bus->impl;
    if(event_bus->queue.size == 0)
        return TRUE;

    event_bus->processing = TRUE;
    bool processed = TRUE;
    for(usz i = 0; i < event_bus->queue.size; i++) {
        EventListener* listener = darray_get_at(&event_bus->queue, i);
        if(!listener) {
            processed = FALSE;
            break;
        }
        EventListener active_listener = *listener;
        active_listener.callback(&active_listener.context);
        if(event_bus->destroy_requested) {
            processed = FALSE;
            break;
        }
    }

    event_bus->processing = FALSE;
    if(event_bus->destroy_requested) {
        event_bus_destroy_now(bus);
        return FALSE;
    }

    darray_clear(&event_bus->queue);
    arena_clear(&event_bus->payload_arena);
    return processed;
}

EventContext event_bus_make_context(EventCode code, void* publisher, void* payload, usz payload_size, usz payload_alignment) {
    return (EventContext) {
        .code              = code,
        .publisher         = publisher,
        .payload           = payload,
        .payload_size      = payload_size,
        .payload_alignment = payload_alignment
    };
}
