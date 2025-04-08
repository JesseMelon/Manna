#include "app.h"
//temporary engine source code link
#include <core/input.h>
#include <memory/memory.h>

b8 initialize_app(manna_app* game_instance) {
    LOG_DEBUG("Game initialized");
    return TRUE;
}

b8 update_app(manna_app* game_instance, f32 delta_time) {
    
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();

    if (is_key_up('M') && was_key_down('M')) {
        LOG_DEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    return TRUE;
}

b8 render_app(manna_app* game_instance, f32 delta_time) {
    return TRUE;
}

void app_on_resize(manna_app* game_instance, u32 width, u32 height) {
    LOG_DEBUG("Game resized");
}
