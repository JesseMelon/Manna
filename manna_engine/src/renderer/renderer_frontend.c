#include "math/math_functions.h"
#include "renderer/renderer_types.h"
#include "renderer_backend.h"
#include "renderer_frontend.h"
#include "core/logger.h"

typedef struct renderer_state {
    renderer_backend backend;
} renderer_state;

static renderer_state* state_ptr;

b8 init_renderer(u64* memory_requirement, void* state, const char *application_name) {
    *memory_requirement = sizeof(renderer_state);
    if (state == 0) {
        return TRUE;
    }

    state_ptr = state;

    //TODO: make configurable
    create_renderer_backend(VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
        LOG_FATAL("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }
    return TRUE;
}

void shutdown_renderer(void* state) {

    if (state_ptr) {
        state_ptr->backend.shutdown(&state_ptr->backend);
    }
    state_ptr = 0;
}

b8 begin_frame(f32 delta_time) {

    if (!state_ptr) {
        return FALSE;
    }

    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 end_frame(f32 delta_time) {

    if (!state_ptr) {
        return FALSE;
    }
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return result;
}

void on_resized(u16 width, u16 height) {
    if (state_ptr) {
        state_ptr->backend.resize(&state_ptr->backend, width, height);
    } else {
        LOG_WARN("renderer backend does not exist to accept resize");
    }
}

b8 draw_frame(render_data *data) {
    if (begin_frame(data->delta_time)) {

        state_ptr->backend.update_global_state(mat4_identity(), mat4_identity(), vec3_zero(), vec4_one(), 0);

        b8 result = end_frame(data->delta_time);

        if (!result) {
            LOG_ERROR("Renderer failed to finish frame. Shutting down.");
            return FALSE;
        }
    }
    return TRUE;
}
