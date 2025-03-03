#include "application.h"
#include "logger.h"
#include "../platform/platform.h"
#include "../core/memory.h"
#include "api_types.h"

typedef struct application_state {
	game* game_instance;
	b8 is_running;
	b8 is_suspended;
	platform_state platform;
	i16 width;
	i16 height;
	f64 last_time;
} application_state;

static b8 is_initialized = FALSE;
static application_state app_state;

b8 application_create(game* game_instance) {
	if (is_initialized) {
		M_ERROR("Application already initialized!");
		return FALSE;
	}

	app_state.game_instance = game_instance;

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

	if (!platform_startup(
		&app_state.platform,
		game_instance->app_config.title,
		game_instance->app_config.x, 
		game_instance->app_config.y,
		game_instance->app_config.width,
		game_instance->app_config.height
	)) {
		return FALSE;
	}

	//initialize game
	if (!app_state.game_instance->initialize(app_state.game_instance)) {
		M_FATAL("Failed to initialize game.");
		return FALSE;
	}

	app_state.game_instance->on_resize(app_state.game_instance, app_state.width, app_state.height);

	is_initialized = TRUE;
	return TRUE;
}

b8 application_run() {
	M_INFO(get_memory_usage());

	while (app_state.is_running) {
		if (!platform_get_messages(&app_state.platform)) return FALSE;

		if (!app_state.is_suspended) {
			if (!app_state.game_instance->update(app_state.game_instance, (f32)0.0)) {
				M_FATAL("Failed to update game.");
				app_state.is_running = FALSE;
				break;
			}

			if (!app_state.game_instance->render(app_state.game_instance, (f32)0.0)) {
				M_FATAL("Failed to render game.");
				app_state.is_running = FALSE;
				break;
			}
		}
	}
	app_state.is_running = FALSE;
	platform_shutdown(&app_state.platform);

	return TRUE;
}
