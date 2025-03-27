#pragma once
#include "defines.h"

typedef union vec2 {
    f32 elements[2];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
    };
} vec2;

//right handed (z out of screen) not that it matters here
typedef union vec3 {
    f32 elements[3];
    struct {
        union {
            f32 x, r, s, u;
        };
    };
    struct {
        union {
            f32 y, g, t, v;
        };
    };
    struct {
        union {
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4 {
    f32 elements[4];
    struct {
        union {
            f32 x, r, s;
        };
        union {
            f32 y, g, t;
        };
        union {
            f32 z, b, p;
        };
        union {
            f32 w, a, q;
        };
    };
} vec4;

typedef vec4 quat;

typedef union mat4_u {
    f32 data[16];
} mat4;

typedef struct vertex {
    vec3 position;
} vertex;
