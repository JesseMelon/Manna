#include <core/logger.h>
#include <math/math_functions.h>

#define expect_equal(expected, actual)                                                                      \
    if (actual != expected) {                                                                               \
        LOG_ERROR("--> Expected %lld, but got: %lld. File: %s:%d", expected, actual, __FILE__, __LINE__);   \
        return FALSE;                                                                                       \
    }

#define expect_not_equal(expected, actual)                                                                  \
    if (actual == expected) {                                                                               \
        LOG_ERROR("--> Expected %lld != %lld. File: %s:%d", expected, actual, __FILE__, __LINE__);          \
        return FALSE;                                                                                       \
    }

#define expect_equal_rounded(expected, actual)                                                              \
    if (m_abs(expected - actual) > 0.001f) {                                                                \
        LOG_ERROR("--> Expected %lld, but got: %lld. File: %s:%d", expected, actual, __FILE__, __LINE__);   \
        return FALSE;                                                                                       \
    }
