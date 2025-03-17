#pragma once

#include "../defines.h"

struct manna_app;

typedef struct application_create_info {
	i16 x;
	i16 y;
	i16 width;
	i16 height;
	char* title;
} application_create_info;

MANNA_API b8 create_application(struct manna_app* app_instance);

MANNA_API b8 run_application();

void application_get_framebuffer_size(u32* width, u32* height);
