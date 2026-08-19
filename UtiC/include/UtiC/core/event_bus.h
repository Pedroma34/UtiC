#pragma once
#include "UtiC/core/types.h"
#include "UtiC/memory/allocator.h"

#define EVENT_BUS_MAX_EVENT_TYPES 200u
#define EVENT_BUS_MAX_LISTENERS_PER_TYPE 24u
#define EVENT_BUS_MAX_QUEUED_DELIVERIES 200u

typedef u64 EventCode;
typedef struct EventContext EventContext;
typedef void(*EventCallback)(EventContext* context);

typedef struct EventContext {
    EventCode code;
    void* subscriber;
    void* publisher;
    void* payload;
    usz payload_size;
    usz payload_alignment;
} EventContext;

typedef struct EventSubscriber {
    EventCode code;
    void* subscriber;
    EventCallback callback;
} EventSubscriber;

typedef struct EventBus {
    Allocator backing_allocator;
    struct EventBusImpl* impl;
} EventBus;

/*
 * Creates a bus in output storage that need not be initialized first.
 * Calling create again on a live bus without destroying it is invalid.
 * EventCode values use the full u64 range; the bus can retain up to
 * EVENT_BUS_MAX_EVENT_TYPES distinct subscribed codes at once.
 */
bool event_bus_create(EventBus* bus, Allocator backing_allocator);
void event_bus_destroy(EventBus* bus);
bool event_bus_subscribe(EventBus* bus, const EventSubscriber* subscriber);
bool event_bus_publish(EventBus* bus, const EventContext* context);
/* Returns FALSE for recursive processing or when a callback destroys the bus. */
bool event_bus_process(EventBus* bus);
EventContext event_bus_make_context(EventCode code, void* publisher, void* payload, usz payload_size, usz payload_alignment);

#define EVENT_MAKE_CONTEXT(usz_code, publisher_ptr, payload_ptr) \
    event_bus_make_context((usz_code), (publisher_ptr),(void*)(payload_ptr), sizeof(*(payload_ptr)), _Alignof(__typeof__(*(payload_ptr))))
