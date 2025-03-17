#include "core/event.h"
#include "platform/platform.h"
#if M_PLATFORM_LINUX
#include "containers/darray.h"
#include "core/input.h"
#include "renderer/vulkan/vulkan_platform.h"
#include <X11/X.h>
#include <xcb/xproto.h>


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

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include "vulkan/vulkan_xcb.h"
#include "renderer/vulkan/vulkan_types.h"

typedef struct platform_state {
    Display* display;   //using Xlib display type
    xcb_connection_t *connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
    VkSurfaceKHR surface;
} platform_state;

static platform_state* state_ptr;

keys translate_keycode(u32 x_keycode);

b8 platform_startup(u64* memory_requirement, void* state, const char* title, i32 x, i32 y, i32 width, i32 height) {
    *memory_requirement = sizeof(platform_state);
    if(state == 0) {
        return TRUE;
    }

    state_ptr = state;

    //connect to X via Xlib
    state_ptr->display = XOpenDisplay(NULL);

    //will not use key repeats. This is global across the OS, so we need to turn it back on later. //TODO look into switching to pure XCB
    XAutoRepeatOff(state_ptr->display);

    state_ptr->connection = XGetXCBConnection(state_ptr->display);

    if (xcb_connection_has_error(state_ptr->connection)) {
        LOG_FATAL("Failed to connect to X server using XCB.");
        return FALSE;
    }

    const struct xcb_setup_t* setup = xcb_get_setup(state_ptr->connection);

    //loop through screens with iterator to get screens. How does this get more than one? 
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    int screen_p = 0;
    for (i32 s = screen_p; s > 0; s--) {
        xcb_screen_next(&it);
    }

    state_ptr->screen = it.data;

    //generate window id
    state_ptr->window = xcb_generate_id(state_ptr->connection);

    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |    //mouse buttons
                        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |         //keyboard
                        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |       //screen max/min and mouse pos
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY;                                //close window events (i think)
    
    //send bg colour and event flags to XCB
    u32 value_list[] = {state_ptr->screen->black_pixel, event_values};

    //create window
    xcb_void_cookie_t cookie = xcb_create_window(
        state_ptr->connection, 
        XCB_COPY_FROM_PARENT,           //depth
        state_ptr->window,                  //window id
        state_ptr->screen->root,            //parent of window
        x, 
        y, 
        width, 
        height, 
        0,                              //no border
        XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class (input and output)
        state_ptr->screen->root_visual,     //id for the windows visual
        event_mask, 
        value_list);

    //apply title
    xcb_change_property(state_ptr->connection, XCB_PROP_MODE_REPLACE, state_ptr->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    //intercept delete events
    //Events to listen for
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(state_ptr->connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(state_ptr->connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    //function pointers to execute
    xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(state_ptr->connection, wm_delete_cookie, NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(state_ptr->connection, wm_protocols_cookie, NULL);

    state_ptr->wm_delete_win = wm_delete_reply->atom;
    state_ptr->wm_protocols = wm_protocols_reply->atom;

    //
    xcb_change_property(state_ptr->connection, XCB_PROP_MODE_REPLACE, state_ptr->window, wm_protocols_reply->atom, 4, 32, 1, &wm_delete_reply->atom);

    xcb_map_window(state_ptr->connection, state_ptr->window);

    //flush stream
    i32 stream_result = xcb_flush(state_ptr->connection);
    if (stream_result <= 0) {
        LOG_FATAL("Error flushing window stream: %d", stream_result);
        return FALSE;
    }

    return TRUE;

}

void shutdown_platform(void* state) {
    if (state_ptr) {
        //turn key repeats back on since it is global for the OS. Cringe.
        XAutoRepeatOn(state_ptr->display);

        xcb_destroy_window(state_ptr->connection, state_ptr->window);
    }
}

b8 platform_get_messages() {

    if (state_ptr) {
        
        xcb_generic_event_t* event;                     //XCB dynamically allocates this event. JEEEEEEZ. 
        xcb_client_message_event_t* client_message;

        b8 quitting = FALSE;

        while (event != 0) {
            event = xcb_poll_for_event(state_ptr->connection);
            if (event == 0) {
                break;
            }

            switch (event->response_type & ~0x80) { //bitwise nonsense is required for basic use of XCB api. weird.
                case XCB_KEY_PRESS:
                case XCB_KEY_RELEASE: {
                    //handle key press and release
                    xcb_key_press_event_t *kb_event = (xcb_key_press_event_t *)event;
                    b8 pressed = event->response_type == XCB_KEY_PRESS;
                    xcb_keycode_t keycode = kb_event->detail;
                    KeySym key_sym = XkbKeycodeToKeysym(state_ptr->display, (KeyCode)keycode, 0, keycode & ShiftMask ? 1 : 0);
                    keys key = translate_keycode(key_sym);
                    process_key(key, pressed);
                } break;
                case XCB_BUTTON_PRESS:
                case XCB_BUTTON_RELEASE: {
                    //handle mouse buttons
                    xcb_button_press_event_t* mouse_event = (xcb_button_press_event_t*)event;
                    b8 pressed = event->response_type == XCB_BUTTON_PRESS;
                    mouse_button mouse_button = MB_COUNT;
                    switch (mouse_event->detail) {
                        case XCB_BUTTON_INDEX_1:
                            mouse_button = MB_LEFT;
                            break;
                        case XCB_BUTTON_INDEX_2:
                            mouse_button = MB_MIDDLE;
                            break;
                        case XCB_BUTTON_INDEX_3:
                            mouse_button = MB_RIGHT;
                            break;
                    }
                    //TODO: make debug
                    if (mouse_button != MB_COUNT) {
                        process_mouse_button(mouse_button, pressed);
                    }
                } break;
                case XCB_MOTION_NOTIFY: {
                    //mouse movement
                    xcb_motion_notify_event_t* move_event = (xcb_motion_notify_event_t*)event;
                    process_mouse_move(move_event->event_x, move_event->event_y);
                } break;
                case XCB_CONFIGURE_NOTIFY: {
                    //window resize
                    xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;
                    event_data data;
                    data.u16[0] = configure_event->width;
                    data.u16[1] = configure_event->height;
                    trigger_event(EVENT_WINDOW_RESIZED, 0, data);

                } break;
                case XCB_CLIENT_MESSAGE: {
                    client_message = (xcb_client_message_event_t*)event;
                    
                    //window close
                    if(client_message->data.data32[0] == state_ptr->wm_delete_win) {
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
    return TRUE;
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

void* platform_copy_memory(void *dst, const void *src, u64 size) {
    return memcpy(dst, src, size);
}

void* platform_set_memory(void *block, u8 value, u64 size) {
    return memset(block, value, size);
}

void platform_console_write(const char *str, u8 color) {
    // FATAL,ERROR,WARN,DEBUG,INFO,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;34", "1;32", "1;30"};    //escape codes 
    printf("\033[%sm%s\033[0m", colour_strings[color], str);
}

//just using stdout still for errors on linux
void platform_console_write_error(const char *str, u8 color) {
    // FATAL,ERROR,WARN,DEBUG,INFO,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;34", "1;32", "1;30"};    //escape codes 
    printf("\033[%sm%s\033[0m", colour_strings[color], str);
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

void vulkan_platform_get_required_extension_names(const char*** names_darray) {
    darray_push(*names_darray, &"VK_KHR_xcb_surface"); //VK_KHR_xlib_surface?
}

b8 platform_create_vulkan_surface(struct vulkan_context *context) {

    if (!state_ptr) {
        return FALSE;
    }

    VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
    create_info.connection = state_ptr->connection;
    create_info.window = state_ptr->window;

    VkResult result = vkCreateXcbSurfaceKHR(
        context->instance, 
        &create_info, 
        context->allocator, 
        &state_ptr->surface);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Vulkan surface creation failed");
        return FALSE;
    }
    context->surface = state_ptr->surface;
    return TRUE;
}

keys translate_keycode(u32 x_keycode) {
    switch (x_keycode) {
         case XK_BackSpace:
             return KEY_BACKSPACE;
         case XK_Return:
             return KEY_ENTER;
         case XK_Tab:
             return KEY_TAB;
             //case XK_Shift: return KEY_SHIFT;
             //case XK_Control: return KEY_CONTROL;
 
         case XK_Pause:
             return KEY_PAUSE;
         case XK_Caps_Lock:
             return KEY_CAPITAL;
 
         case XK_Escape:
             return KEY_ESCAPE;
 
             // Not supported
             // case : return KEY_CONVERT;
             // case : return KEY_NONCONVERT;
             // case : return KEY_ACCEPT;
 
         case XK_Mode_switch:
             return KEY_MODECHANGE;
 
         case XK_space:
             return KEY_SPACE;
         case XK_Prior:
             return KEY_PRIOR;
         case XK_Next:
             return KEY_NEXT;
         case XK_End:
             return KEY_END;
         case XK_Home:
             return KEY_HOME;
         case XK_Left:
             return KEY_LEFT;
         case XK_Up:
             return KEY_UP;
         case XK_Right:
             return KEY_RIGHT;
         case XK_Down:
             return KEY_DOWN;
         case XK_Select:
             return KEY_SELECT;
         case XK_Print:
             return KEY_PRINT;
         case XK_Execute:
             return KEY_EXECUTE;
         // case XK_snapshot: return KEY_SNAPSHOT; // not supported
         case XK_Insert:
             return KEY_INSERT;
         case XK_Delete:
             return KEY_DELETE;
         case XK_Help:
             return KEY_HELP;
 
         case XK_Meta_L:
             return KEY_LWIN;  // TODO: not sure this is right
         case XK_Meta_R:
             return KEY_RWIN;
             // case XK_apps: return KEY_APPS; // not supported
 
             // case XK_sleep: return KEY_SLEEP; //not supported
 
         case XK_KP_0:
             return KEY_NUMPAD0;
         case XK_KP_1:
             return KEY_NUMPAD1;
         case XK_KP_2:
             return KEY_NUMPAD2;
         case XK_KP_3:
             return KEY_NUMPAD3;
         case XK_KP_4:
             return KEY_NUMPAD4;
         case XK_KP_5:
             return KEY_NUMPAD5;
         case XK_KP_6:
             return KEY_NUMPAD6;
         case XK_KP_7:
             return KEY_NUMPAD7;
         case XK_KP_8:
             return KEY_NUMPAD8;
         case XK_KP_9:
             return KEY_NUMPAD9;
         case XK_multiply:
             return KEY_MULTIPLY;
         case XK_KP_Add:
             return KEY_ADD;
         case XK_KP_Separator:
             return KEY_SEPARATOR;
         case XK_KP_Subtract:
             return KEY_SUBTRACT;
         case XK_KP_Decimal:
             return KEY_DECIMAL;
         case XK_KP_Divide:
             return KEY_DIVIDE;
         case XK_F1:
             return KEY_F1;
         case XK_F2:
             return KEY_F2;
         case XK_F3:
             return KEY_F3;
         case XK_F4:
             return KEY_F4;
         case XK_F5:
             return KEY_F5;
         case XK_F6:
             return KEY_F6;
         case XK_F7:
             return KEY_F7;
         case XK_F8:
             return KEY_F8;
         case XK_F9:
             return KEY_F9;
         case XK_F10:
             return KEY_F10;
         case XK_F11:
             return KEY_F11;
         case XK_F12:
             return KEY_F12;
         case XK_F13:
             return KEY_F13;
         case XK_F14:
             return KEY_F14;
         case XK_F15:
             return KEY_F15;
         case XK_F16:
             return KEY_F16;
         case XK_F17:
             return KEY_F17;
         case XK_F18:
             return KEY_F18;
         case XK_F19:
             return KEY_F19;
         case XK_F20:
             return KEY_F20;
         case XK_F21:
             return KEY_F21;
         case XK_F22:
             return KEY_F22;
         case XK_F23:
             return KEY_F23;
         case XK_F24:
             return KEY_F24;
 
         case XK_Num_Lock:
             return KEY_NUMLOCK;
         case XK_Scroll_Lock:
             return KEY_SCROLL;
 
         case XK_KP_Equal:
             return KEY_NUMPAD_EQUAL;
 
         case XK_Shift_L:
             return KEY_LSHIFT;
         case XK_Shift_R:
             return KEY_RSHIFT;
         case XK_Control_L:
             return KEY_LCONTROL;
         case XK_Control_R:
             return KEY_RCONTROL;
         case XK_Alt_L:
             return KEY_LALT;
         case XK_Alt_R:
             return KEY_RALT;
 
         case XK_semicolon:
             return KEY_SEMICOLON;
         case XK_plus:
             return KEY_PLUS;
         case XK_comma:
             return KEY_COMMA;
         case XK_minus:
             return KEY_MINUS;
         case XK_period:
             return KEY_PERIOD;
         case XK_slash:
             return KEY_SLASH;
         case XK_grave:
             return KEY_GRAVE;
 
         case XK_a:
         case XK_A:
             return KEY_A;
         case XK_b:
         case XK_B:
             return KEY_B;
         case XK_c:
         case XK_C:
             return KEY_C;
         case XK_d:
         case XK_D:
             return KEY_D;
         case XK_e:
         case XK_E:
             return KEY_E;
         case XK_f:
         case XK_F:
             return KEY_F;
         case XK_g:
         case XK_G:
             return KEY_G;
         case XK_h:
         case XK_H:
             return KEY_H;
         case XK_i:
         case XK_I:
             return KEY_I;
         case XK_j:
         case XK_J:
             return KEY_J;
         case XK_k:
         case XK_K:
             return KEY_K;
         case XK_l:
         case XK_L:
             return KEY_L;
         case XK_m:
         case XK_M:
             return KEY_M;
         case XK_n:
         case XK_N:
             return KEY_N;
         case XK_o:
         case XK_O:
             return KEY_O;
         case XK_p:
         case XK_P:
             return KEY_P;
         case XK_q:
         case XK_Q:
             return KEY_Q;
         case XK_r:
         case XK_R:
             return KEY_R;
         case XK_s:
         case XK_S:
             return KEY_S;
         case XK_t:
         case XK_T:
             return KEY_T;
         case XK_u:
         case XK_U:
             return KEY_U;
         case XK_v:
         case XK_V:
             return KEY_V;
         case XK_w:
         case XK_W:
             return KEY_W;
         case XK_x:
         case XK_X:
             return KEY_X;
         case XK_y:
         case XK_Y:
             return KEY_Y;
         case XK_z:
         case XK_Z:
             return KEY_Z;
 
         default:
             return 0;
     }
 }

#endif
