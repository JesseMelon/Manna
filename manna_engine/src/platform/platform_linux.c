#include "platform.h"
#include <xcb/xproto.h>

#if M_PLATFORM_LINUX

#include "core/logger.h"

//in linux, windowing uses a client-server architecture. X11 is the server, Xlib and Xcb (newer) are clients
//somewhat popular is an Xlib/Xcb hybrid
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

    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)platform_state->internal_state;

    //connect to X
    state->display = XOpenDisplay(NULL);

    //will not use key repeats. This is global across the OS, so we need to turn it back on later.
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
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(state->connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(state->connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(state->connection, wm_delete_cookie, NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(state->connection, wm_protocols_cookie, NULL);
    state->wm_delete_win = wm_delete_reply->atom;
    state->wm_protocols = wm_protocols_reply->atom;

}

void platform_shutdown(platform_state *state) {

}

b8 platform_get_messages(platform_state *state) {

}

void* platform_allocate(u64, b8 aligned) {

}

void platform_free(void *block, b8 aligned) {

}

void* platform_zero_memory(void *block, u64 size) {
    
}

void* platform_copy_memory(void *dst, void *src, u64 size) {

}

void* platform_set_memory(void *block, u8 value, u64 size) {

}

void platform_console_write(const char *str, u8 color) {

}

void platform_console_write_error(const char *str, u8 color) {

}

f64 platform_get_time() {

}

void platform_sleep(u64 ms) {

}

#endif
