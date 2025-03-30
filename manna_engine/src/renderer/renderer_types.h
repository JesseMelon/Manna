#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum renderer_backend_type {
    VULKAN,
    OPENGL,
    DIRECTX
} renderer_backend_type;

//align to 256 bytes (some nvidia devices apparently require this)
typedef struct global_uniform_object {
    mat4 projection;    //64 bytes
    mat4 view;          //64 bytes
    mat4 reserved0;     //64 bytes
    mat4 reserved1;     //64 bytes
} global_uniform_object;

typedef struct renderer_backend {
    struct platform_state* platform_state;
    u64 frame_number;
    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resize)(struct renderer_backend* backend, u16 width, u16 height);
    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_object)(mat4 model);
} renderer_backend;

typedef struct render_data {
    f32 delta_time;
} render_data;
