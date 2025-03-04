#include "platform/platform.h"

#if M_PLATFORM_LINUX

#include "core/logger.h"

//in linux, windowing uses a client-server architecture. X11 is the server, Xlib and Xcb (newer) are clients
//somewhat popular is an Xlib/Xcb hybrid

//TODO revisit all of this with Wayland 
#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h> //sudo apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h> //sudo apt-get install libxkbcommon-x11-dev and libx11-xcb-dev
//different distributions use different sleep functions

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct internal_state {
    Display* display;   //using Xlib display type
    xcb_connection_t *connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
} internal_state;

b8 platform_startup(platform_state* platform_state, const char* title, i32 x, i32 y, i32 width, i32 height) {

    platform_state->internal_state = malloc(sizeof(internal_state)); //memory exists for life of app
    internal_state* state = (internal_state*)platform_state->internal_state;

    //connect to X via Xlib, TODO look into switching to pure XCB
    state->display = XOpenDisplay(NULL);

    //will not use key repeats. This is global across the OS, so we need to turn it back on later. //TODO look into switching to pure XCB
    XAutoRepeatOff(state->display);

    state->connection = XGetXCBConnection(state->display);

    if (xcb_connection_has_error(state->connection)) {
        LOG_FATAL("Failed to connect to X server using XCB.");
        return FALSE;
    }

    const struct xcb_setup_t* setup = xcb_get_setup(state->connection);

    //loop through screens with iterator to get screens. How does this get more than one? 
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    int screen_p = 0;
    for (i32 s = screen_p; s > 0; s--) {
        xcb_screen_next(&it);
    }

    state->screen = it.data;

    //generate window id
    state->window = xcb_generate_id(state->connection);

    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |    //mouse buttons
                        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |         //keyboard
                        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |       //screen max/min and mouse pos
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY;                                //close window events (i think)
    
    //send bg colour and event flags to XCB
    u32 value_list[] = {state->screen->black_pixel, event_values};

    //create window
    xcb_void_cookie_t cookie = xcb_create_window(
        state->connection, 
        XCB_COPY_FROM_PARENT,           //depth
        state->window,                  //window id
        state->screen->root,            //parent of window
        x, 
        y, 
        width, 
        height, 
        0,                              //no border
        XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class (input and output)
        state->screen->root_visual,     //id for the windows visual
        event_mask, 
        value_list);

    //apply title
    xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE, state->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    //intercept delete events
    //Events to listen for
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(state->connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(state->connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    //function pointers to execute
    xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(state->connection, wm_delete_cookie, NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(state->connection, wm_protocols_cookie, NULL);

    state->wm_delete_win = wm_delete_reply->atom;
    state->wm_protocols = wm_protocols_reply->atom;

    //
    xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE, state->window, wm_protocols_reply->atom, 4, 32, 1, &wm_delete_reply->atom);

    xcb_map_window(state->connection, state->window);

    //flush stream
    i32 stream_result = xcb_flush(state->connection);
    if (stream_result <= 0) {
        LOG_FATAL("Error flushing window stream: %d", stream_result);
        return FALSE;
    }

    return TRUE;

}

void platform_shutdown(platform_state* platform_state) {
    internal_state* state = (internal_state*) platform_state->internal_state;

    //turn key repeats back on since it is global for the OS. Cringe.
    XAutoRepeatOn(state->display);

    xcb_destroy_window(state->connection, state->window);
}

b8 platform_get_messages(platform_state *platform_state) {
    internal_state* state = (internal_state*)platform_state->internal_state;

    xcb_generic_event_t* event;                     //XCB dynamically allocates this event. JEEEEEEZ. 
    xcb_client_message_event_t* client_message;

    b8 quitting = FALSE;

    while (event != 0) {
        event = xcb_poll_for_event(state->connection);
        if (event == 0) {
            break;
        }

        switch (event->response_type & ~0x80) { //bitwise nonsense is required for basic use of XCB api. weird.
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
                //handle key press and release
            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
                //handle mouse buttons
            } break;
            case XCB_MOTION_NOTIFY: {
                //mouse movement
            } break;
            case XCB_CONFIGURE_NOTIFY: {
                //window resize
            } break;
            case XCB_CLIENT_MESSAGE: {
                client_message = (xcb_client_message_event_t*)event;
                
                //window close
                if(client_message->data.data32[0] == state->wm_delete_win) {
                    quitting = TRUE;
                }
            } break;
            default:
                break;
        }

        free(event);
    }
    return !quitting;
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void *ptr, b8 aligned) {
    free(ptr);
}

void* platform_zero_memory(void *block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void *dst, void *src, u64 size) {
    return memcpy(dst, src, size);
}

void* platform_set_memory(void *block, u8 value, u64 size) {
    return memset(block, value, size);
}

void platform_console_write(const char *str, u8 color) {
    // FATAL,ERROR,WARN,DEBUG,INFO,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;34", "1;32", "1;30"};    //escape codes 
    printf("\033[0m");
    printf("\033[%sm%s\033[0m", colour_strings[color], str);
    printf("DEBUG: After color %d\n", color);  // Confirm reset
    fflush(stdout);
}

//just using stdout still for errors on linux
void platform_console_write_error(const char *str, u8 color) {
    // FATAL,ERROR,WARN,DEBUG,INFO,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;34", "1;32", "1;30"};    //escape codes 
    printf("\033[0m");
    printf("\033[%sm%s\033[0m", colour_strings[color], str);
    printf("DEBUG: After color %d\n", color);  // Confirm reset
    fflush(stdout);
}

f64 platform_get_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(u64 ms) {
    //ubuntu tends to use the top option here
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts,0);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

#endif
