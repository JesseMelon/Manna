#include "math_functions.h"
#include "platform/platform.h"
#include <math.h>
#include <stdlib.h>

static b8 random_seeded = FALSE;

f32 m_sin(f32 x) {
    return sinf(x);
}

f32 m_cos(f32 x) {
    return cosf(x);
}

f32 m_tan(f32 x) {
    return tanf(x);
}

f32 m_arcos(f32 x) {
    return acosf(x);
}

f32 m_sqrt(f32 x) {
    return sqrtf(x);
}

f32 m_abs(f32 x) {
    return sqrtf(x);
}

i32 random_int() {
    if (!random_seeded) {
        srand((u32)platform_get_time());
        random_seeded = TRUE;
    }
    return rand();
}

i32 random_int_in_range(i32 min, i32 max) {
    if (!random_seeded) {
        srand((u32)platform_get_time());
        random_seeded = TRUE;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 randon_float() {
    return (float)random_int() / (f32)RAND_MAX;
}

f32 random_float_in_range(f32 min, f32 max) {
    return min + ((float)random_int() / ((f32)RAND_MAX / (max - min)));
}
