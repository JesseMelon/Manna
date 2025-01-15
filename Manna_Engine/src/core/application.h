#pragma once

#include "../defines.h"

typedef struct application_create_info {
	i16 x, y, width, height;
	char* title;
} application_create_info;

MANNA_API b8 create_application(application_create_info* config);

MANNA_API b8 run_application();