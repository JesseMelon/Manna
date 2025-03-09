#pragma once

#include "platform/platform.h"
#include "renderer_types.h"

struct static_mesh_data;
struct platform_state;
b8 init_renderer(const char* application_name, struct platform_state* platform_state);
void shutdown_renderer();

void on_resized(u16 width, u16 height);

b8 draw_frame(render_data* data);
