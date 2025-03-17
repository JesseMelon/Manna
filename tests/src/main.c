#include "test_manager.h"

#include "memory/linear_allocator_tests.h"

#include <core/logger.h>

int main() {
    init_test_manager();

    linear_allocator_register_tests();

    LOG_DEBUG("Starting tests.");

    test_manager_run_tests();

    return 0;
}
