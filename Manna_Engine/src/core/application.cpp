#include "application.h"
#include "logger.h"
#include "platform/platform.h"

typedef struct application_state {
	b8 is_running;
	b8 is_suspended;
	platform_state platform;
	i16 width;
	i16 height;
	f64 last_time;
} application_state;

static b8 is_initialized = FALSE;
static application_state app_state;

b8 create_application(application_create_info* config) {
	if (is_initialized) {
		M_ERROR("Application already initialized!");
		return FALSE;
	}
	//init subsystems
	init_logger();

	//TODO: remove this
	M_FATAL("Fatal! %f:", 1.0);
	M_ERROR("Error!");
	M_WARN("Warn!");
	M_DEBUG("Debug!");
	M_INFO("Info!");
	M_TRACE("Trace!");

	app_state.is_running = TRUE;
	app_state.is_suspended = FALSE;

	if (!platform_startup(&app_state.platform, config->title, config->x, config->y, config->width, config->height)) {
		return FALSE;
	}

	is_initialized = TRUE;
	return TRUE;
}

b8 run_application() {
	while (app_state.is_running) {
		if(!platform_update(&app_state.platform)) return FALSE;
	}

	app_state.is_running = FALSE;
	platform_shutdown(&app_state.platform);

	return TRUE;
}