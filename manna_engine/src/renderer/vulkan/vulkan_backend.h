#pragma once

#include "renderer/renderer_backend.h"
#include "renderer/renderer_types.h"

b8 init_vulkan_renderer_backend(renderer_backend* backend, const char* application_name, struct platform_state* platform_state);
void shutdown_vulkan_renderer_backend(renderer_backend* backend);

void vulkan_renderer_backend_on_resize(renderer_backend* backend, u16 width, u16 height);

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time);
b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time);
