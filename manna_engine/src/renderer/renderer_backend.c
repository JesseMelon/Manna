#include "renderer/renderer_backend.h"
#include "platform/platform.h"
#include "renderer/renderer_types.h"

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* platform_state, renderer_backend* out_renderer_backend) {
    out_renderer_backend->platform_state = platform_state;

    if (type == VULKAN) {

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
