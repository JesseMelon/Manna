#include "linear_allocator_tests.h"
#include "../test_manager.h"
#include "../expect.h"

#include <defines.h>
#include <memory/linear_allocator.h>

u8 lin_alloc_create_destroy() {
    linear_allocator allocator;
    create_linear_allocator(sizeof(u64), 0, &allocator);
    expect_not_equal(0, allocator.memory);
    expect_equal(sizeof(u64), allocator.total_size);
    expect_equal(0, allocator.allocated);

    destroy_linear_allocator(&allocator);

    expect_not_equal(0, allocator.memory);
    expect_equal(0, allocator.total_size);
    expect_equal(0, allocator.allocated);

    return TRUE;
}

u8 lin_alloc_multi_alloc_all_space() {
    u64 max_allocs = 1024;
    linear_allocator allocator;
    create_linear_allocator(sizeof(u64) * max_allocs, 0, &allocator);
    
    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&allocator, sizeof(u64));
        expect_not_equal(0, block);
        expect_equal(sizeof(u64) * (i + 1), allocator.allocated);
    }

    destroy_linear_allocator(&allocator);
    
    return TRUE;
}

u8 lin_alloc_all_space() {
    linear_allocator allocator;
    create_linear_allocator(sizeof(u64), 0, &allocator);

    void* block = linear_allocator_allocate(&allocator, sizeof(u64));

    expect_not_equal(0, block);
    expect_equal(sizeof(u64), allocator.allocated);

    destroy_linear_allocator(&allocator);

    return TRUE;
}

u8 lin_alloc_over_allocate() {
    u64 max_allocs = 3;
    linear_allocator allocator;
    create_linear_allocator(sizeof(u64) * max_allocs, 0, &allocator);

    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&allocator, sizeof(u64));
        expect_not_equal(0, block);
        expect_equal(sizeof(u64) * (i + 1), allocator.allocated);
    }

    LOG_DEBUG("Note: The next error is intentionally caused");
    block = linear_allocator_allocate(&allocator, sizeof(u64));
    expect_equal(0, block);
    expect_equal(sizeof(u64) * (max_allocs), allocator.allocated);

    destroy_linear_allocator(&allocator);

    return TRUE;
}

u8 lin_alloc_full_then_free() {
    u64 max_allocs = 1024;
    linear_allocator allocator;
    create_linear_allocator(sizeof(u64) * max_allocs, 0, &allocator);

    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&allocator, sizeof(u64));
        expect_not_equal(0, block);
        expect_equal(sizeof(u64) * (i + 1), allocator.allocated);
    }

    linear_allocator_free_all(&allocator);
    expect_equal(0, allocator.allocated);
    destroy_linear_allocator(&allocator);

    return TRUE;

}

void linear_allocator_register_tests() {
    test_manager_register_test(lin_alloc_create_destroy, "Linear allocator create and destroy");
    test_manager_register_test(lin_alloc_all_space, "Linear allocator single alloc for all space");
    test_manager_register_test(lin_alloc_multi_alloc_all_space, "Linear allocator multi alloc for all space");
    test_manager_register_test(lin_alloc_over_allocate, "Linear allocator attempt to over-allocate");
    test_manager_register_test(lin_alloc_full_then_free, "Linear allocator free all");
}
