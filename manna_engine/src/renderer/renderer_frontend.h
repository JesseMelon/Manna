#pragma once

#include "renderer_types.h"

b8 init_renderer(u64* memory_requirement, void* state, const char* application_name);
void shutdown_renderer(void* state);

void on_resized(u16 width, u16 height);

b8 draw_frame(render_data* data);
