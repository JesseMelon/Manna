#pragma once

#include "../defines.h"

struct game;

typedef struct application_create_info {
	i16 x;
	i16 y;
	i16 width;
	i16 height;
	char* title;
} application_create_info;

MANNA_API b8 application_create(struct game* game_instance);

MANNA_API b8 application_run();