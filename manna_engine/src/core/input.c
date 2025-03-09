#include "core/input.h"
#include "core/event.h"
#include "core/memory.h"
#include "core/logger.h"

//TODO: move all the checks if initialized to a debug macro

typedef struct keyboard_state {
    b8 keys[256]; //why not make this bitwise? 
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    u8 buttons[MB_COUNT];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

static b8 initialized = FALSE;
static input_state state = {};

void init_input() {
    initialized = TRUE;
    LOG_INFO("Input subsystem initialized");
}

void shutdown_input() {
    initialized = FALSE;
}

void update_input(f64 delta_time) {
    if (!initialized) {
        return;
    }
    m_copy_memory(&state.keyboard_previous, &state.keyboard_current, sizeof(keyboard_state));
    m_copy_memory(&state.mouse_previous, &state.mouse_current, sizeof(mouse_state));
}

//can poll keyboard state or listen for events
void process_key(keys key, b8 pressed) {
    if (state.keyboard_current.keys[key] != pressed) {
        state.keyboard_current.keys[key] = pressed;
        event_data event_data;
        event_data.u16[0] = key;
        trigger_event(pressed ? EVENT_KEY_PRESSED : EVENT_KEY_RELEASED, 0, event_data);
    }
}

void process_mouse_button(mouse_button button, b8 pressed) {
    if (state.mouse_current.buttons[button] != pressed) {
        state.mouse_current.buttons[button] = pressed;
        event_data event_data;
        event_data.u16[0] = button;
        trigger_event(pressed ? EVENT_MOUSE_BUTTON_PRESSED : EVENT_MOUSE_BUTTON_RELEASED, 0, event_data);
    }
}

void process_mouse_move(i16 x, i16 y) {
    if (state.mouse_current.x != x || state.mouse_current.y != y) {
        //LOG_DEBUG("Mouse pos: %i, %i", x, y);
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        event_data event_data;
        event_data.u16[0] = x;
        event_data.u16[1] = y;
        trigger_event(EVENT_MOUSE_MOVED, 0, event_data);
    }
}

void process_mouse_wheel(i8 z_delta) {
    event_data event_data;
    event_data.u8[0] = z_delta;
    trigger_event(EVENT_MOUSE_WHEEL, 0, event_data);
}

b8 is_key_down(keys key) {
    if (!initialized) {
        return FALSE;
    }
    return state.keyboard_current.keys[key] == FALSE;
}

b8 is_key_up(keys key) {
    if (!initialized) {
        return TRUE;
    }
    return state.keyboard_current.keys[key] == TRUE;
}

b8 was_key_down(keys key) {
    if (!initialized) {
        return FALSE;
    }
    return state.keyboard_previous.keys[key] == FALSE;
}

b8 was_key_up(keys key) {
    if (!initialized) {
        return TRUE;
    }
    return state.keyboard_previous.keys[key] == TRUE;
}

b8 is_mouse_button_down(mouse_button button) {
    if (!initialized) {
        return FALSE;
    }
    return state.mouse_current.buttons[button] == TRUE;
}

b8 is_mouse_button_up(mouse_button button) {
    if (!initialized) {
        return TRUE;
    }
    return state.mouse_current.buttons[button] == FALSE;
}

b8 was_mouse_button_down(mouse_button button) {
    if (!initialized) {
        return FALSE;
    }
    return state.mouse_previous.buttons[button] == TRUE;
}

b8 was_mouse_button_up(mouse_button button) {
    if (!initialized) {
        return TRUE;
    }
    return state.mouse_previous.buttons[button] == FALSE;
}

void get_mouse_position(i32 *x, i32 *y) {
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void get_previous_mouse_position(i32 *x, i32 *y) {
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}
