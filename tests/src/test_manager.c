#include "test_manager.h"

#include <containers/darray.h>
#include <core/logger.h>
#include <core/mstring.h>
#include <core/clock.h>

typedef struct test_entry {
    test_function func;
    char* desc;
} test_entry;

static test_entry* tests;

void init_test_manager() {
    tests = darray_create(test_entry);
}

void test_manager_register_test(test_function func, char *desc) {
    test_entry e;
    e.func = func;
    e.desc = desc;
    darray_push(tests, e)
}

void test_manager_run_tests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = darray_get_length(tests);

    clock total_time;
    start_clock(&total_time);

    for (u32 i = 0; i < count; ++i) {
        clock test_time;
        start_clock(&test_time);
        u8 result = tests[i].func();
        update_clock(&test_time);
        if (result == TRUE) {
            ++passed;
        } else if (result == BYPASS) {
            LOG_WARN("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        }else {
            LOG_ERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        format_string(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        update_clock(&total_time);
        LOG_INFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    stop_clock(&total_time);

    LOG_INFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}
