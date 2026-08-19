#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <UtiC/core/event_bus.h>
#include <UtiC/memory/allocator.h>
#include <UtiC/memory/allocator_counter.h>
#include <UtiC/memory/allocator_system.h>
#include <UtiC/memory/arena.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if(!(condition)) {                                                      \
            fprintf(                                                           \
                stderr,                                                         \
                "%s:%d: check failed: %s\n",                                   \
                __FILE__,                                                       \
                __LINE__,                                                       \
                #condition                                                      \
            );                                                                  \
            return FALSE;                                                       \
        }                                                                       \
    } while(0)

typedef struct BusFixture {
    AllocatorCounter counter;
    EventBus bus;
} BusFixture;

typedef struct CountState {
    usz calls;
    EventCode last_code;
} CountState;

typedef struct FailAllocatorState {
    Allocator upstream;
    usz attempts;
    usz fail_on_attempt;
} FailAllocatorState;

static bool fixture_create(BusFixture* fixture) {
    if(!fixture)
        return FALSE;

    *fixture = (BusFixture) { 0 };
    fixture->counter = allocator_counter_create(system_allocator(), NULL, NULL);
    Allocator allocator = allocator_counter_get_allocator(&fixture->counter);
    return event_bus_create(&fixture->bus, allocator);
}

static bool fixture_destroy(BusFixture* fixture) {
    if(!fixture)
        return FALSE;

    event_bus_destroy(&fixture->bus);
    return fixture->bus.impl == NULL && fixture->counter.info.live_allocations == 0;
}

static void count_callback(EventContext* context) {
    if(!context || !context->subscriber)
        return;

    CountState* state = (CountState*)context->subscriber;
    state->calls++;
    state->last_code = context->code;
}

static void* fail_allocator_alloc(usz size, usz alignment, void* user) {
    FailAllocatorState* state = (FailAllocatorState*)user;
    if(!state)
        return NULL;

    state->attempts++;
    if(state->attempts == state->fail_on_attempt)
        return NULL;
    return allocator_alloc(&state->upstream, size, alignment);
}

static void fail_allocator_free(void* block, usz size, usz alignment, void* user) {
    FailAllocatorState* state = (FailAllocatorState*)user;
    if(!state)
        return;
    allocator_free(&state->upstream, block, size, alignment);
}

static bool test_publish_atomic_at_resource_limit(void) {
    enum { DISCOVERY_LIMIT = 1024, PAYLOAD_SIZE = 256 };
    const EventCode filler_code = UINT64_C(10);
    const EventCode target_code = UINT64_C(11);
    static byte payload[PAYLOAD_SIZE];

    BusFixture fixture;
    CountState filler = { 0 };
    CountState target_a = { 0 };
    CountState target_b = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber filler_subscriber = {
        .code = filler_code,
        .subscriber = &filler,
        .callback = count_callback
    };
    EventSubscriber target_subscriber_a = {
        .code = target_code,
        .subscriber = &target_a,
        .callback = count_callback
    };
    EventSubscriber target_subscriber_b = {
        .code = target_code,
        .subscriber = &target_b,
        .callback = count_callback
    };
    CHECK(event_bus_subscribe(&fixture.bus, &filler_subscriber));
    CHECK(event_bus_subscribe(&fixture.bus, &target_subscriber_a));
    CHECK(event_bus_subscribe(&fixture.bus, &target_subscriber_b));

    EventContext filler_context = event_bus_make_context(
        filler_code,
        NULL,
        payload,
        sizeof(payload),
        _Alignof(byte)
    );
    EventContext target_context = event_bus_make_context(
        target_code,
        NULL,
        payload,
        sizeof(payload),
        _Alignof(byte)
    );

    usz capacity = 0;
    while(capacity < DISCOVERY_LIMIT && event_bus_publish(&fixture.bus, &filler_context))
        capacity++;

    CHECK(capacity >= 2);
    CHECK(capacity < DISCOVERY_LIMIT);
    CHECK(event_bus_process(&fixture.bus));
    CHECK(filler.calls == capacity);

    filler.calls = 0;
    for(usz i = 0; i < capacity - 1; i++)
        CHECK(event_bus_publish(&fixture.bus, &filler_context));

    const bool target_published = event_bus_publish(&fixture.bus, &target_context);
    CHECK(event_bus_process(&fixture.bus));
    CHECK(filler.calls == capacity - 1);
    CHECK(
        (target_published && target_a.calls == 1 && target_b.calls == 1) ||
        (!target_published && target_a.calls == 0 && target_b.calls == 0)
    );

    if(!target_published) {
        CHECK(event_bus_publish(&fixture.bus, &target_context));
        CHECK(event_bus_process(&fixture.bus));
        CHECK(target_a.calls == 1);
        CHECK(target_b.calls == 1);
    }

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

static bool test_publish_atomic_on_payload_exhaustion(void) {
    const EventCode target_code = UINT64_C(20);
    const EventCode recovery_code = UINT64_C(21);
    static byte large_payload[KB(30)];
    static byte recovery_payload[KB(48)];

    BusFixture fixture;
    CountState target_a = { 0 };
    CountState target_b = { 0 };
    CountState recovery = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber subscribers[] = {
        { target_code, &target_a, count_callback },
        { target_code, &target_b, count_callback },
        { recovery_code, &recovery, count_callback }
    };
    for(usz i = 0; i < sizeof(subscribers) / sizeof(subscribers[0]); i++)
        CHECK(event_bus_subscribe(&fixture.bus, &subscribers[i]));

    EventContext target_context = event_bus_make_context(
        target_code,
        NULL,
        large_payload,
        sizeof(large_payload),
        _Alignof(byte)
    );
    const bool target_published = event_bus_publish(&fixture.bus, &target_context);
    CHECK(event_bus_process(&fixture.bus));
    CHECK(
        (target_published && target_a.calls == 1 && target_b.calls == 1) ||
        (!target_published && target_a.calls == 0 && target_b.calls == 0)
    );

    EventContext recovery_context = event_bus_make_context(
        recovery_code,
        NULL,
        recovery_payload,
        sizeof(recovery_payload),
        _Alignof(byte)
    );
    CHECK(event_bus_publish(&fixture.bus, &recovery_context));
    CHECK(event_bus_process(&fixture.bus));
    CHECK(recovery.calls == 1);

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

static bool test_null_descriptors_are_rejected(void) {
    const EventCode code = UINT64_C(30);
    u32 payload = 42;
    BusFixture fixture;
    CountState state = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber subscriber = { code, &state, count_callback };
    EventContext context = event_bus_make_context(
        code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );

    CHECK(!event_bus_subscribe(&fixture.bus, NULL));
    CHECK(!event_bus_publish(&fixture.bus, NULL));
    CHECK(!event_bus_subscribe(NULL, &subscriber));
    CHECK(!event_bus_publish(NULL, &context));
    CHECK(!event_bus_process(NULL));
    event_bus_destroy(NULL);

    CHECK(event_bus_subscribe(&fixture.bus, &subscriber));
    CHECK(event_bus_publish(&fixture.bus, &context));
    CHECK(event_bus_process(&fixture.bus));
    CHECK(state.calls == 1);

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

static bool test_create_accepts_uninitialized_output(void) {
    AllocatorCounter counter = allocator_counter_create(system_allocator(), NULL, NULL);
    Allocator allocator = allocator_counter_get_allocator(&counter);

    EventBus uninitialized_bus;
    CHECK(event_bus_create(&uninitialized_bus, allocator));
    event_bus_destroy(&uninitialized_bus);
    CHECK(uninitialized_bus.impl == NULL);
    CHECK(counter.info.live_allocations == 0);

    EventBus poisoned_bus;
    memset(&poisoned_bus, 0xA5, sizeof(poisoned_bus));
    CHECK(event_bus_create(&poisoned_bus, allocator));
    event_bus_destroy(&poisoned_bus);
    CHECK(poisoned_bus.impl == NULL);
    CHECK(counter.info.live_allocations == 0);

    EventBus invalid_allocator_bus;
    memset(&invalid_allocator_bus, 0xA5, sizeof(invalid_allocator_bus));
    CHECK(!event_bus_create(&invalid_allocator_bus, (Allocator) { 0 }));
    CHECK(invalid_allocator_bus.impl == NULL);
    event_bus_destroy(&invalid_allocator_bus);

    FailAllocatorState fail_state = {
        .upstream = allocator,
        .attempts = 0,
        .fail_on_attempt = 1
    };
    Allocator failing_allocator = {
        .alloc = fail_allocator_alloc,
        .free = fail_allocator_free,
        .user = &fail_state
    };
    EventBus failed_bus;
    memset(&failed_bus, 0xA5, sizeof(failed_bus));
    CHECK(!event_bus_create(&failed_bus, failing_allocator));
    CHECK(failed_bus.impl == NULL);
    event_bus_destroy(&failed_bus);
    CHECK(counter.info.live_allocations == 0);

    CHECK(!event_bus_create(NULL, allocator));
    return TRUE;
}

typedef struct AnonymousState {
    usz callback_a_calls;
    usz callback_b_calls;
} AnonymousState;

static void anonymous_callback_a(EventContext* context) {
    if(context && context->publisher)
        ((AnonymousState*)context->publisher)->callback_a_calls++;
}

static void anonymous_callback_b(EventContext* context) {
    if(context && context->publisher)
        ((AnonymousState*)context->publisher)->callback_b_calls++;
}

static void named_callback_a(EventContext* context) {
    if(context && context->subscriber)
        ((AnonymousState*)context->subscriber)->callback_a_calls++;
}

static void named_callback_b(EventContext* context) {
    if(context && context->subscriber)
        ((AnonymousState*)context->subscriber)->callback_b_calls++;
}

static bool test_subscriber_identity_includes_callback(void) {
    const EventCode anonymous_code = UINT64_C(40);
    const EventCode named_code = UINT64_C(41);
    u32 payload = 7;
    BusFixture fixture;
    AnonymousState anonymous = { 0 };
    AnonymousState named = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber anonymous_a = { anonymous_code, NULL, anonymous_callback_a };
    EventSubscriber anonymous_b = { anonymous_code, NULL, anonymous_callback_b };
    CHECK(event_bus_subscribe(&fixture.bus, &anonymous_a));
    CHECK(event_bus_subscribe(&fixture.bus, &anonymous_b));
    CHECK(!event_bus_subscribe(&fixture.bus, &anonymous_a));

    EventSubscriber named_a = { named_code, &named, named_callback_a };
    EventSubscriber named_b = { named_code, &named, named_callback_b };
    CHECK(event_bus_subscribe(&fixture.bus, &named_a));
    CHECK(event_bus_subscribe(&fixture.bus, &named_b));
    CHECK(!event_bus_subscribe(&fixture.bus, &named_a));

    EventContext anonymous_context = event_bus_make_context(
        anonymous_code,
        &anonymous,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    EventContext named_context = event_bus_make_context(
        named_code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    CHECK(event_bus_publish(&fixture.bus, &anonymous_context));
    CHECK(event_bus_publish(&fixture.bus, &named_context));
    CHECK(event_bus_process(&fixture.bus));
    CHECK(anonymous.callback_a_calls == 1);
    CHECK(anonymous.callback_b_calls == 1);
    CHECK(named.callback_a_calls == 1);
    CHECK(named.callback_b_calls == 1);

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

static bool test_event_codes_use_full_u64_range(void) {
    const EventCode low_code = UINT64_C(0);
    const EventCode high_code = UINT64_C(0xfedcba9876543210);
    u32 payload = 99;
    BusFixture fixture;
    CountState low = { 0 };
    CountState high = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber low_subscriber = { low_code, &low, count_callback };
    EventSubscriber high_subscriber = { high_code, &high, count_callback };
    CHECK(event_bus_subscribe(&fixture.bus, &low_subscriber));
    CHECK(event_bus_subscribe(&fixture.bus, &high_subscriber));

    EventContext high_context = event_bus_make_context(
        high_code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    EventContext low_context = event_bus_make_context(
        low_code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    CHECK(event_bus_publish(&fixture.bus, &high_context));
    CHECK(event_bus_publish(&fixture.bus, &low_context));
    CHECK(event_bus_process(&fixture.bus));
    CHECK(low.calls == 1);
    CHECK(low.last_code == low_code);
    CHECK(high.calls == 1);
    CHECK(high.last_code == high_code);

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

typedef struct RecursiveProcessState {
    EventBus* bus;
    usz calls;
    bool nested_result;
} RecursiveProcessState;

static void recursive_process_callback(EventContext* context) {
    if(!context || !context->subscriber)
        return;

    RecursiveProcessState* state = (RecursiveProcessState*)context->subscriber;
    state->calls++;
    state->nested_result = event_bus_process(state->bus);
}

static bool test_recursive_process_is_rejected(void) {
    const EventCode code = UINT64_C(50);
    u32 payload = 1;
    BusFixture fixture;
    CHECK(fixture_create(&fixture));

    RecursiveProcessState state = {
        .bus = &fixture.bus,
        .calls = 0,
        .nested_result = TRUE
    };
    EventSubscriber subscriber = { code, &state, recursive_process_callback };
    EventContext context = event_bus_make_context(
        code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    CHECK(event_bus_subscribe(&fixture.bus, &subscriber));
    CHECK(event_bus_publish(&fixture.bus, &context));
    CHECK(event_bus_process(&fixture.bus));
    CHECK(state.calls == 1);
    CHECK(!state.nested_result);
    CHECK(event_bus_process(&fixture.bus));

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

typedef struct DestroyState {
    EventBus* bus;
    usz calls;
} DestroyState;

static void destroy_bus_callback(EventContext* context) {
    if(!context || !context->subscriber)
        return;

    DestroyState* state = (DestroyState*)context->subscriber;
    state->calls++;
    event_bus_destroy(state->bus);
}

static bool test_destroy_during_process_is_safe(void) {
    const EventCode code = UINT64_C(60);
    u32 payload = 1;
    BusFixture fixture;
    CountState later = { 0 };
    CHECK(fixture_create(&fixture));

    DestroyState destroy_state = { &fixture.bus, 0 };
    EventSubscriber destroy_subscriber = { code, &destroy_state, destroy_bus_callback };
    EventSubscriber later_subscriber = { code, &later, count_callback };
    EventContext context = event_bus_make_context(
        code,
        NULL,
        &payload,
        sizeof(payload),
        _Alignof(u32)
    );
    CHECK(event_bus_subscribe(&fixture.bus, &destroy_subscriber));
    CHECK(event_bus_subscribe(&fixture.bus, &later_subscriber));
    CHECK(event_bus_publish(&fixture.bus, &context));
    CHECK(!event_bus_process(&fixture.bus));
    CHECK(destroy_state.calls == 1);
    CHECK(later.calls == 0);
    CHECK(fixture.bus.impl == NULL);
    CHECK(fixture.counter.info.live_allocations == 0);

    event_bus_destroy(&fixture.bus);
    CHECK(fixture.counter.info.live_allocations == 0);
    return TRUE;
}

static bool test_arena_empty_contract(void) {
    AllocatorCounter counter = allocator_counter_create(system_allocator(), NULL, NULL);
    Allocator allocator = allocator_counter_get_allocator(&counter);
    Arena arena = { 0 };

    CHECK(arena_is_empty(NULL));
    CHECK(arena_create(&arena, allocator, 128));
    CHECK(arena_is_empty(&arena));

    Allocator arena_allocator = arena_get_allocator(&arena);
    CHECK(allocator_alloc(&arena_allocator, sizeof(u64), _Alignof(u64)) != NULL);
    CHECK(!arena_is_empty(&arena));
    arena_clear(&arena);
    CHECK(arena_is_empty(&arena));

    arena_destroy(&arena);
    CHECK(arena_is_empty(&arena));
    CHECK(counter.info.live_allocations == 0);
    return TRUE;
}

static bool test_event_payload_arena_is_reused(void) {
    const EventCode code = UINT64_C(70);
    static byte payload[KB(48)];
    BusFixture fixture;
    CountState state = { 0 };
    CHECK(fixture_create(&fixture));

    EventSubscriber subscriber = { code, &state, count_callback };
    EventContext context = event_bus_make_context(
        code,
        NULL,
        payload,
        sizeof(payload),
        _Alignof(byte)
    );
    CHECK(event_bus_subscribe(&fixture.bus, &subscriber));

    for(usz i = 0; i < 4; i++) {
        CHECK(event_bus_publish(&fixture.bus, &context));
        CHECK(event_bus_process(&fixture.bus));
        CHECK(state.calls == i + 1);
    }

    CHECK(fixture_destroy(&fixture));
    return TRUE;
}

static bool test_zero_state_cleanup_is_safe(void) {
    EventBus bus = { 0 };
    Arena arena = { 0 };

    event_bus_destroy(&bus);
    event_bus_destroy(&bus);
    arena_destroy(&arena);
    arena_destroy(&arena);
    CHECK(bus.impl == NULL);
    CHECK(arena.buffer == NULL);
    return TRUE;
}

typedef bool (*TestFunction)(void);

typedef struct TestCase {
    const char* name;
    TestFunction function;
} TestCase;

static const TestCase TEST_CASES[] = {
    { "publish_atomic_at_resource_limit", test_publish_atomic_at_resource_limit },
    { "publish_atomic_on_payload_exhaustion", test_publish_atomic_on_payload_exhaustion },
    { "null_descriptors_are_rejected", test_null_descriptors_are_rejected },
    { "create_accepts_uninitialized_output", test_create_accepts_uninitialized_output },
    { "subscriber_identity_includes_callback", test_subscriber_identity_includes_callback },
    { "event_codes_use_full_u64_range", test_event_codes_use_full_u64_range },
    { "recursive_process_is_rejected", test_recursive_process_is_rejected },
    { "destroy_during_process_is_safe", test_destroy_during_process_is_safe },
    { "arena_empty_contract", test_arena_empty_contract },
    { "event_payload_arena_is_reused", test_event_payload_arena_is_reused },
    { "zero_state_cleanup_is_safe", test_zero_state_cleanup_is_safe }
};

static int run_test(const TestCase* test_case) {
    if(!test_case->function()) {
        fprintf(stderr, "FAILED: %s\n", test_case->name);
        return 1;
    }
    fprintf(stdout, "PASSED: %s\n", test_case->name);
    return 0;
}

int main(int argc, char** argv) {
    const usz test_count = sizeof(TEST_CASES) / sizeof(TEST_CASES[0]);

    if(argc == 1) {
        for(usz i = 0; i < test_count; i++) {
            if(run_test(&TEST_CASES[i]) != 0)
                return 1;
        }
        return 0;
    }

    if(argc != 2) {
        fprintf(stderr, "usage: %s [test-name]\n", argv[0]);
        return 2;
    }

    for(usz i = 0; i < test_count; i++) {
        if(strcmp(argv[1], TEST_CASES[i].name) == 0)
            return run_test(&TEST_CASES[i]);
    }

    fprintf(stderr, "unknown test: %s\n", argv[1]);
    return 2;
}
