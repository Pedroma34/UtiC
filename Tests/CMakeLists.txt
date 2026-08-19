add_executable(
    UtiCEventBusTests
    "event_bus_tests.c"
)

set_target_properties(
    UtiCEventBusTests
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

target_link_libraries(
    UtiCEventBusTests
    PRIVATE
        UtiC
)

set(
    UTIC_EVENT_BUS_TEST_CASES
    publish_atomic_at_resource_limit
    publish_atomic_on_payload_exhaustion
    null_descriptors_are_rejected
    create_accepts_uninitialized_output
    subscriber_identity_includes_callback
    event_codes_use_full_u64_range
    recursive_process_is_rejected
    destroy_during_process_is_safe
    arena_empty_contract
    event_payload_arena_is_reused
    zero_state_cleanup_is_safe
)

foreach(TEST_CASE IN LISTS UTIC_EVENT_BUS_TEST_CASES)
    add_test(
        NAME "UtiC.EventBus.${TEST_CASE}"
        COMMAND UtiCEventBusTests "${TEST_CASE}"
    )
    set_tests_properties(
        "UtiC.EventBus.${TEST_CASE}"
        PROPERTIES
            LABELS "UtiC;event_bus"
            TIMEOUT 10
    )
endforeach()

if(TARGET Testbed)
    add_test(
        NAME UtiC.Examples.TestbedSmoke
        COMMAND Testbed
    )
    set_tests_properties(
        UtiC.Examples.TestbedSmoke
        PROPERTIES
            LABELS "UtiC;example"
            TIMEOUT 10
    )
endif()
