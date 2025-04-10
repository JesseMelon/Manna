#pragma once
#include "core/application.h"

typedef struct game {

	application_create_info app_config;

	b8(*initialize)(struct game* game_instance);

	b8(*update)(struct game* game_instance, f32 delta_time);

	b8(*render)(struct game* game_instance, f32 delta_time);

	void(*on_resize)(struct game* game_instance, u32 width, u32 height);

	void* state;

} game;