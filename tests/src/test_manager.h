#pragma once

#include <defines.h>

#define BYPASS 2

typedef u8 (*test_function)();

void init_test_manager();

void test_manager_register_test(test_function func, char* desc);

void test_manager_run_tests();
