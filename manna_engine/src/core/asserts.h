#pragma once

#include "../defines.h"

#define M_ASSERTIONS_ENABLED

#ifdef M_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

MANNA_API void report_assertion_fail(const char* file, int line, const char* expression, const char* message);

#define M_ASSERT(expr, msg) if (expr) {} else { report_assertion_fail(__FILE__, __LINE__, #expr, msg); debugBreak(); }

#ifdef _DEBUG
#define M_ASSERT_DEBUG(expr, msg) if (expr) {} else { report_assertion_fail(__FILE__, __LINE__, #expr, msg); debugBreak(); } //for assertions made in debug builds only
#else
#define M_ASSERT_DEBUG(expr, msg)
#endif

#else
#define M_ASSERT(expr, msg)
#define M_ASSERT_DEBUG(expr, msg)
#endif