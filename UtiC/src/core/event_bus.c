#include <string.h> /* memcpy */
#include "UtiC/core/event_bus.h"
#include "UtiC/memory/arena.h"
#include "UtiC/containers/darray.h"

typedef struct EventListener {
    EventCallback callback;
    EventContext context;
} EventListener;

#define EVENT_LISTENERS_MAX_SIZE 24
typedef struct Event {
    EventListener listeners[EVENT_LISTENERS_MAX_SIZE];
    usz listeners_count;
} Event;

#define EVENT_ARENA_SIZE KB(56)
#define EVENT_REGISTRY_MAX_SIZE 200llu /* Estimate number of events for the client application */
#define EVENT_QUEUE_MAX_SIZE 200llu /* Estimate per frame */
typedef struct EventBusImpl {
    Event* registry;
    DArray queue; /* EventListener array. Gonna make sure it doesn't resize tho. Could've just made it a pointer and a counter but fuck it  */
    Arena payload_arena;
} EventBusImpl;

static void _event_bus_destroy(EventBus* bus) {
    if(!bus || !bus->impl)
        return;
    
    if(bus->impl->registry) {
        ALLOCATOR_FREE_ARRAY(Event, &bus->backing_allocator, bus->impl->registry, EVENT_REGISTRY_MAX_SIZE);
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
    if(!bus || bus->impl || !backing_allocator.alloc || !backing_allocator.free)
        return FALSE;

    *bus = (EventBus) { 0 };
    bus->impl = ALLOCATOR_ALLOC_ZEROED(EventBusImpl, &backing_allocator);
    bus->backing_allocator = backing_allocator;
    if(!bus->impl)
        return FALSE;

    EventBusImpl* evnt_bus = bus->impl;
    evnt_bus->registry = ALLOCATOR_ALLOC_ARRAY_ZEROED(Event, &backing_allocator, EVENT_REGISTRY_MAX_SIZE);
    if(!evnt_bus->registry) {
        _event_bus_destroy(bus);
        return FALSE;
    }
    if(!DARRAY_CREATE(EventListener, &evnt_bus->queue, backing_allocator, EVENT_QUEUE_MAX_SIZE)) {
        _event_bus_destroy(bus);
        return FALSE;
    }

    if(!arena_create(&evnt_bus->payload_arena, backing_allocator, EVENT_ARENA_SIZE)) {
        _event_bus_destroy(bus);
        return FALSE;
    }

    return TRUE;
}

void event_bus_destroy(EventBus *bus) {
    _event_bus_destroy(bus);
}

bool event_bus_subscribe(EventBus* bus, const EventSubscriber* subscriber) {
    if(!bus || !bus->impl)
        return FALSE;
    if(subscriber->code >= EVENT_REGISTRY_MAX_SIZE || !subscriber->callback)
        return FALSE;
        
    EventBusImpl* event_bus = bus->impl;
    Event* event = &event_bus->registry[subscriber->code];
    if(event->listeners_count >= EVENT_LISTENERS_MAX_SIZE)
        return FALSE;

    /* Looking for duplicates */
    for(usz i = 0; i < event->listeners_count; i++) {
        if(event->listeners[i].context.subscriber == subscriber->subscriber)
            return FALSE;
    }

    EventListener* new_listener = &event->listeners[event->listeners_count];
    new_listener->callback      = subscriber->callback;
    new_listener->context.subscriber = subscriber->subscriber;
    new_listener->context.code       = subscriber->code;
    event->listeners_count++;
    return TRUE;
}

bool event_bus_publish(EventBus *bus, const EventContext *context) {
    if(!bus || !bus->impl || !context->payload || context->payload_size == 0)
        return FALSE;
    /* Power of two */
    if(context->payload_alignment && ((context->payload_alignment & (context->payload_alignment - 1)) != 0))
        return FALSE;
    if(context->code >= EVENT_REGISTRY_MAX_SIZE)
        return FALSE;

    EventBusImpl* event_bus = bus->impl;
    Event* event = &event_bus->registry[context->code];
	if (event->listeners_count == 0)
	    return FALSE;

    for(usz i = 0; i < event->listeners_count; i++) {
        EventListener* listener = &event->listeners[i];
        if(event_bus->queue.size >= EVENT_QUEUE_MAX_SIZE)
            return FALSE;
       EventListener queued_listener     = *listener;
	   queued_listener.context.publisher = context->publisher;
       Allocator arena_allocator         = arena_get_allocator(&event_bus->payload_arena);
	   queued_listener.context.payload   = allocator_alloc(&arena_allocator, context->payload_size, context->payload_alignment);
       if(!queued_listener.context.payload)
           return FALSE;
        queued_listener.context.payload_size      = context->payload_size;
		queued_listener.context.payload_alignment = context->payload_alignment;
		memcpy(queued_listener.context.payload, context->payload, context->payload_size);
        EventListener* listener_queue = darray_push_zeroed(&event_bus->queue);
		if(!listener_queue)
            return FALSE;
		*listener_queue = queued_listener;
    }

    return TRUE;
}


bool event_bus_process(EventBus* bus) {
    if(!bus || !bus->impl)
        return FALSE;
    EventBusImpl* event_bus = bus->impl;
    if(event_bus->queue.size == 0)
        return TRUE;

    for(usz i = 0; i < event_bus->queue.size; i++) {
        EventListener* listener = darray_get_at(&event_bus->queue, i);
        if(!listener)
            return FALSE;
        listener->callback(&listener->context);
    }

    darray_clear(&event_bus->queue);
    if(arena_is_empty(&event_bus->payload_arena))
        arena_clear(&event_bus->payload_arena); /* Only resetting if there's shit in it */
    return TRUE;
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