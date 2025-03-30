#include "renderer/renderer_backend.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_backend.h"

b8 create_renderer_backend(renderer_backend_type type, renderer_backend* out_renderer_backend) {

    if (type == VULKAN) {
        out_renderer_backend->initialize = init_vulkan_renderer_backend;
        out_renderer_backend->shutdown = shutdown_vulkan_renderer_backend;
        out_renderer_backend->begin_frame = vulkan_renderer_backend_begin_frame;
        out_renderer_backend->update_global_state = vulkan_renderer_update_global_state;
        out_renderer_backend->end_frame = vulkan_renderer_backend_end_frame;
        out_renderer_backend->resize = vulkan_renderer_backend_on_resize;
        out_renderer_backend->update_object = vulkan_backend_update_object;

        return TRUE;
    }

    return FALSE;
}

void renderer_backend_destroy(renderer_backend* renderer_backend) {
    renderer_backend->initialize = 0;
    renderer_backend->shutdown = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->update_global_state = 0;
    renderer_backend->end_frame = 0;
    renderer_backend->resize = 0;
    renderer_backend->update_object = 0;
}
