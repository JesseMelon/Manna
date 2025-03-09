#include "renderer/renderer_backend.h"
#include "platform/platform.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_backend.h"

b8 create_renderer_backend(renderer_backend_type type, struct platform_state* platform_state, renderer_backend* out_renderer_backend) {
    out_renderer_backend->platform_state = platform_state;

    if (type == VULKAN) {
        out_renderer_backend->initialize = init_vulkan_renderer_backend;
        out_renderer_backend->shutdown = shutdown_vulkan_renderer_backend;
        out_renderer_backend->begin_frame = vulkan_renderer_backend_begin_frame;
        out_renderer_backend->end_frame = vulkan_renderer_backend_end_frame;
        out_renderer_backend->resize = vulkan_renderer_backend_on_resize;

        return TRUE;
    }

    return FALSE;
}

void renderer_backend_destroy(renderer_backend* renderer_backend) {
    renderer_backend->initialize = 0;
    renderer_backend->shutdown = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->end_frame = 0;
    renderer_backend->resize = 0;
}
