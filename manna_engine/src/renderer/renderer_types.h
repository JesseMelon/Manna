#pragma once

#include "defines.h"

typedef enum renderer_backend_type {
    VULKAN,
    OPENGL,
    DIRECTX
} renderer_backend_type;

typedef struct renderer_backend {
    struct platform_state* platform_state;
    u64 frame_number;
    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resize)(struct renderer_backend* backend, u16 width, u16 height);
    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
} renderer_backend;

typedef struct render_data {
    f32 delta_time;
} render_data;
