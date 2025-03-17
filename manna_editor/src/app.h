#pragma once

#include <m_defines.h>

typedef struct state {
	f32 delta_time;
} state;

b8 initialize_app(manna_app* game_instance);

b8 update_app(manna_app* game_instance, f32 delta_time);

b8 render_app(manna_app* game_instance, f32 delta_time);

void app_on_resize(manna_app* game_instance, u32 width, u32 height);
