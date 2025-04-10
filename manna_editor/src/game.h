#pragma once

#include <m_defines.h>

typedef struct game_state {
	f32 delta_time;
} game_state;

b8 game_initialize(game* game_instance);

b8 game_update(game* game_instance, f32 delta_time);

b8 game_render(game* game_instance, f32 delta_time);

void game_on_resize(game* game_instance, u32 width, u32 height);