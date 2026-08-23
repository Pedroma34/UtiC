#include <stdio.h>
#include <UtiC/core/event_bus.h>
#include <UtiC/memory/allocator.h>
#include <UtiC/memory/allocator_counter.h>
#include <UtiC/memory/allocator_system.h>
#include <UtiC/memory/arena.h>
#include <UtiC/containers/darray.h>
#include <UtiC/string/cstr.h>
#include <UtiC/io/console.h>
#include <UtiC/core/randy.h>

struct Person {
    const char* name;
    usz age;
};

static void callback(EventContext* context) {
    const struct Person* payload = context->payload;
    fprintf(stdout, "%s %zu\n", payload->name, payload->age);
}

static double format_bytes(usz bytes, const char** unit) {
    static const char* units[] = { "bytes", "KB", "MB", "GB" };
    double value = (double)bytes;
    int index = 0;
    while (value >= 1024.0 && index < 3) {
        value /= 1024.0;
        ++index;
    }
    *unit = units[index];
    return value;
}

static void counter_callback(AllocatorCounterEvent event, const AllocatorCounterInfo* info, usz size, void* callback_user) {
    FILE* output = (FILE*)callback_user;
    const char* event_name = event == ALLOCATOR_COUNTER_EVENT_ALLOC ? "Allocation" : "Deallocation";
    const char* size_unit;
    const char* live_unit;
    double formatted_size = format_bytes(size, &size_unit);
    double formatted_live = format_bytes(info->live_bytes, &live_unit);

    fprintf(output, "%s happened: %.2f %s (%.2f %s live)\n", event_name, formatted_size, size_unit, formatted_live, live_unit);
}

i32 main(void) {
    AllocatorCounter counter = allocator_counter_create(system_allocator(), counter_callback, stdout);
    Arena arena = { 0 };
    EventBus event_bus = { 0 };
    Randy randy = { 0 };
    if(!arena_create(&arena, allocator_counter_get_allocator(&counter), MB(1))) {
        printf("Arena creation failed\n");
        goto cleanup;
    }
    if(!event_bus_create(&event_bus, arena_get_allocator(&arena))) {
        printf("Event Bus creation failed\n");
        goto cleanup;
    }
    randy = randy_init(randy_get_seed_time());

    EventSubscriber sub;
    sub = (EventSubscriber) {
        .callback = callback,
        .code = 69,
        .subscriber = NULL
    };
    if(!event_bus_subscribe(&event_bus, &sub))
        goto cleanup;
    
    struct Person person = (struct Person) {
        .name = "Peter",
        .age = 27
    };
    EventContext ctx = EVENT_MAKE_CONTEXT(69, NULL, &person);
    if(!event_bus_publish(&event_bus, &ctx))
        goto cleanup;

    event_bus_process(&event_bus);
    
    console_write("This is from the console API\n");
    console_writef("This is a %s message!\n", "formatted");

    console_write("Generating numbers...\n");
    i32 numbers[3] = { 0 };
    for(usz i = 0; i < sizeof(numbers) / sizeof(numbers[0]); i++) {
        numbers[i] = RANDY_RANGE(i32, &randy, -10, 10);
    }
    console_writef("Numbers: %d | %d | %d\n", numbers[0], numbers[1], numbers[2]);

cleanup:
    event_bus_destroy(&event_bus);
    const char* arena_size_str;
    double formatted_size = format_bytes(arena.size, &arena_size_str);
    printf("Arena being destroyed. It had: %.2f %s\n", formatted_size, arena_size_str);
    arena_destroy(&arena);
}
