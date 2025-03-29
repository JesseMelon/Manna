#include "application.h"
#include "core/input.h"
#include "logger.h"
#include "platform/platform.h"
#include "memory/memory.h"
#include "core/event.h"
#include "api_types.h"
#include "core/clock.h"
#include "memory/linear_allocator.h"

#include "renderer/renderer_frontend.h"
#include "renderer/renderer_types.h"

typedef struct application_state {
	manna_app* instance;
	b8 is_running;
	b8 is_suspended;
	i16 width;
	i16 height;
    clock clock;
	f64 last_time;
    linear_allocator sys_allocator;
    
    u64 event_system_memory;
    void* event_state;

    u64 memory_system_memory;
    void* memory_state;

    u64 logger_system_memory;
    void* logger_state;

    u64 input_system_memory;
    void* input_state;

    u64 platform_system_memory;
    u64* platform_state;

    u64 renderer_system_memory;
    u64* renderer_state;
} application_state;

static application_state* app_state;

//event handlers
b8 application_on_event(u16 event_id, void* sender, void* listener_instance, event_data data);
b8 application_on_key(u16 event_id, void* sender, void* listener_instance, event_data data);
b8 application_on_resized(u16 event_id, void* sender, void* listener_instance, event_data);

b8 create_application(manna_app* app_instance) {
	if (app_instance->application_state) {
		LOG_ERROR("Application already initialized!");
		return FALSE;
	}

    app_instance->application_state = m_allocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    app_state = app_instance->application_state;
	app_state->instance = app_instance;
    app_state->is_running = FALSE;
	app_state->is_suspended = FALSE;

    u64 sys_allocator_total_size = 32 * 1024 * 1024; //32 mb
    create_linear_allocator(sys_allocator_total_size, 0, &app_state->sys_allocator);

    //init subsystems

    //events
    init_events(&app_state->event_system_memory, 0);
    app_state->event_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->event_system_memory);
    init_events(&app_state->event_system_memory, app_state->event_state);

    //memory
	init_memory(&app_state->memory_system_memory, 0); //lowest level subsystem init to begin tracking memory usage
    app_state->memory_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->memory_system_memory);
    init_memory(&app_state->memory_system_memory, app_state->memory_state);

    //logging
	init_logger(&app_state->logger_system_memory, 0);
    app_state->logger_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->logger_system_memory);
    if(!init_logger(&app_state->logger_system_memory, app_state->logger_state)) {
        LOG_ERROR("Failed to init logger.");
        return FALSE;
    }

    //input
    init_input(&app_state->input_system_memory, 0);
    app_state->input_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->input_system_memory);
    init_input(&app_state->input_system_memory, app_state->input_state);

    listen_to_event(EVENT_APPLICATION_QUIT, 0, application_on_event);
    listen_to_event(EVENT_KEY_PRESSED, 0, application_on_key);
    listen_to_event(EVENT_KEY_RELEASED, 0, application_on_key);
    listen_to_event(EVENT_WINDOW_RESIZED, 0, application_on_resized);

    //platform
    platform_startup(&app_state->platform_system_memory, 0, 0, 0, 0, 0, 0);
    app_state->platform_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->platform_system_memory);
	if (!platform_startup(
		&app_state->platform_system_memory,
        app_state->platform_state,
		app_instance->app_config.title,
		app_instance->app_config.x, 
		app_instance->app_config.y,
		app_instance->app_config.width,
		app_instance->app_config.height
	)) {
		return FALSE;
	}
    
    //renderer
    init_renderer(&app_state->renderer_system_memory, 0, 0);
    app_state->renderer_state = linear_allocator_allocate(&app_state->sys_allocator, app_state->renderer_system_memory);
    if (!init_renderer(&app_state->renderer_system_memory, app_state->renderer_state, app_instance->app_config.title)) {
        LOG_FATAL("Failed to initialize renderer.");
        return FALSE;
    }

	//initialize game
	if (!app_state->instance->initialize(app_state->instance)) {
		LOG_FATAL("Failed to initialize game.");
		return FALSE;
	}
    
    //resize once to ensure proper size
	app_state->instance->on_resize(app_state->instance, app_state->width, app_state->height);

	return TRUE;
}

b8 run_application() {
  	app_state->is_running = TRUE;
    start_clock(&app_state->clock);
    update_clock(&app_state->clock);
    app_state->last_time = app_state->clock.elapsed;

    f64 uptime = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

	LOG_INFO(get_memory_usage());

	while (app_state->is_running) {
		if (!platform_get_messages()) return FALSE;

		if (!app_state->is_suspended) {

            update_clock(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta = (current_time - app_state->last_time);
            f64 frame_start_time = platform_get_time();   

			if (!app_state->instance->update(app_state->instance, (f32)delta)) {
				LOG_FATAL("Failed to update game.");
				app_state->is_running = FALSE;
				break;
			}

			if (!app_state->instance->render(app_state->instance, (f32)delta)) {
				LOG_FATAL("Failed to render game.");
				app_state->is_running = FALSE;
				break;
			}
            
            //TODO: render data
            render_data render_data = {.delta_time = delta};
            draw_frame(&render_data);

            f64 frame_end_time = platform_get_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            uptime += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);
                b8 limit_frames = FALSE;
                if (remaining_ms > 0 && limit_frames) {
                    platform_sleep(remaining_ms - 1);
                }
                frame_count++;
            }

            //NOTE: things which rely on the state of input should be before the update input, update input sets up next frame.
            update_input(delta);

            app_state->last_time = current_time; //update last frame time.
		}
	}
	app_state->is_running = FALSE;
    
    ignore_event(EVENT_APPLICATION_QUIT, 0, application_on_event);
    ignore_event(EVENT_KEY_PRESSED, 0, application_on_key);
    ignore_event(EVENT_KEY_RELEASED, 0, application_on_key);
    shutdown_input(app_state->input_state);
    shutdown_renderer(app_state->renderer_state);
	shutdown_platform(&app_state->platform_state);
    shutdown_events(app_state->event_state);
    shutdown_memory(&app_state->memory_state);


	return TRUE;
}

void application_get_framebuffer_size(u32 *width, u32 *height) {
    *width = app_state->width;
    *height = app_state->height;
}

b8 application_on_event(u16 event_id, void* sender, void* listener_instance, event_data data) {
    switch (event_id) {
        case EVENT_APPLICATION_QUIT:
            LOG_INFO("Application Quit event received, shutting down.\n");
            app_state->is_running = FALSE;
            return TRUE;
    }
    return FALSE;
}

b8 application_on_key(u16 event_id, void* sender, void* listener_instance, event_data data) {
    if (event_id == EVENT_KEY_PRESSED) {
        u16 key = data.u16[0];
        if (key == KEY_ESCAPE) {
            event_data event_data = {};
            trigger_event(EVENT_APPLICATION_QUIT, 0, event_data);

            return TRUE; //means this will not get handled again by anything else;
        } else if (key == KEY_A) {
            LOG_DEBUG("A key recognized");
        } else {
            LOG_DEBUG("'%c' key pressed in window", key);
        }
    } else if (event_id == EVENT_KEY_RELEASED) {
        u16 key = data.u16[0];
        if (key == KEY_B) {
            LOG_DEBUG("B key release recognized");
        } else {
            LOG_DEBUG("'%c' key released in window", key);
        }
    }
    return FALSE;
}

b8 application_on_resized(u16 event_id, void* sender, void* listener_instance, event_data data) {
    if (event_id == EVENT_WINDOW_RESIZED) {
        u16 width = data.u16[0];
        u16 height = data.u16[1];

        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;
            LOG_DEBUG("Window resize: %i, %i", width, height);

            if (width == 0 || height == 0) {
                LOG_INFO("Window minimized, suspending.");
                app_state->is_suspended = TRUE;
                return TRUE;
            } else {
                if (app_state->is_suspended) {
                    LOG_INFO("Window restored, resuming application");
                    app_state->is_suspended = FALSE;
                }
                app_state->instance->on_resize(app_state->instance, width, height);
                //renderer
                on_resized(width, height);
            }
        }
    }
    return FALSE;
}
