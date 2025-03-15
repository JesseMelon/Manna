#include "renderer/renderer_types.h"
#include "renderer_backend.h"
#include "renderer_frontend.h"
#include "core/logger.h"
#include "core/memory.h"

static renderer_backend* backend = 0;

b8 init_renderer(const char *application_name, struct platform_state *platform_state) {
    backend = m_allocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    //TODO: make configurable
    create_renderer_backend(VULKAN, platform_state, backend);
    backend ->frame_number = 0;

    if (!backend->initialize(backend, application_name, platform_state)) {
        LOG_FATAL("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }
    return TRUE;
}

void shutdown_renderer() {
    backend->shutdown(backend);
    m_free(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 begin_frame(f32 delta_time) {
    return backend->begin_frame(backend, delta_time);
}

b8 end_frame(f32 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    backend->frame_number++;
    return result;
}

void on_resized(u16 width, u16 height) {
    if (backend) {
        backend->resize(backend, width, height);
    } else {
        LOG_WARN("renderer backend does not exist to accept resize");
    }
}

b8 draw_frame(render_data *data) {
    if (begin_frame(data->delta_time)) {
        b8 result = end_frame(data->delta_time);

        if (!result) {
            LOG_ERROR("Renderer failed to finish frame. Shutting down.");
            return FALSE;
        }
    }
    return TRUE;
}
