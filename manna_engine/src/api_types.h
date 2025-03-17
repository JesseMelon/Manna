#pragma once
#include "core/application.h"

typedef struct manna_app {

	application_create_info app_config;

	b8(*initialize)(struct manna_app* app_instance);

	b8(*update)(struct manna_app* app_instance, f32 delta_time);

	b8(*render)(struct manna_app* app_instance, f32 delta_time);

	void(*on_resize)(struct manna_app* app_instance, u32 width, u32 height);

	void* state;

    //game owning state data helps enable hot reloading (in exe, not dll)
    void* application_state;
} manna_app;
