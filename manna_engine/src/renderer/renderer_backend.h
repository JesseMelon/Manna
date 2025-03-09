#pragma once

#include "platform/platform.h"
#include "renderer_types.h"

struct platform_state;

b8 create_renderer_backend(renderer_backend_type type, struct platform_state* platform_state, renderer_backend* out_renderer_backend);
void destroy_renderer_backend(renderer_backend* renderer_backend);
