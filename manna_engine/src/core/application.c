#include "application.h"
#include "core/input.h"
#include "logger.h"
#include "platform/platform.h"
#include "core/memory.h"
#include "core/event.h"
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
		LOG_ERROR("Application already initialized!");
		return FALSE;
	}

	app_state.game_instance = game_instance;

	//init subsystems
	init_logger();
    init_input();

	//TODO: remove this
	LOG_FATAL("Fatal! %f:", 1.0);
	LOG_ERROR("Error!");
	LOG_WARN("Warn!");
	LOG_DEBUG("Debug!");
	LOG_INFO("Info!");
	LOG_TRACE("Trace!");

	app_state.is_running = TRUE;
	app_state.is_suspended = FALSE;

    if (!init_events()) {
        LOG_ERROR("Event system failed to initialize, quitting.");
        return FALSE;
    }

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
		LOG_FATAL("Failed to initialize game.");
		return FALSE;
	}

	app_state.game_instance->on_resize(app_state.game_instance, app_state.width, app_state.height);

	is_initialized = TRUE;
	return TRUE;
}

b8 application_run() {
	LOG_INFO(get_memory_usage());

	while (app_state.is_running) {
		if (!platform_get_messages(&app_state.platform)) return FALSE;

		if (!app_state.is_suspended) {
			if (!app_state.game_instance->update(app_state.game_instance, (f32)0.0)) {
				LOG_FATAL("Failed to update game.");
				app_state.is_running = FALSE;
				break;
			}

			if (!app_state.game_instance->render(app_state.game_instance, (f32)0.0)) {
				LOG_FATAL("Failed to render game.");
				app_state.is_running = FALSE;
				break;
			}
            //NOTE: things which rely on the state of input should be before the update input, update input sets up next frame;
            update_input(0);
		}
	}
	app_state.is_running = FALSE;

    shutdown_events();
    shutdown_input();

	platform_shutdown(&app_state.platform);

	return TRUE;
}
