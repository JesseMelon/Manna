#include "core/input.h"
#include "core/event.h"
#include "memory/memory.h"
#include "core/logger.h"

//TODO: move all the checks if initialized to a debug macro

typedef struct keyboard_state {
    b8 keys[256]; //make this bitwise? 
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

static input_state* state_ptr = {};

void init_input(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(input_state);
    if (state == 0) {
        return;
    }
    m_set_memory(state, 0, sizeof(input_state));
    state_ptr = state;
    LOG_INFO("Input subsystem initialized");
}

void shutdown_input(void* state) {
    state_ptr = 0;
}

void update_input(f64 delta_time) {
    if (!state_ptr) {
        return;
    }
    m_copy_memory(&state_ptr->keyboard_previous, &state_ptr->keyboard_current, sizeof(keyboard_state));
    m_copy_memory(&state_ptr->mouse_previous, &state_ptr->mouse_current, sizeof(mouse_state));
}

//can poll keyboard state or listen for events
void process_key(keys key, b8 pressed) {
    
    if (key == KEY_LSHIFT) {
        LOG_INFO("Left shift pressed.");
    } else if (key == KEY_RSHIFT) {
        LOG_INFO("Right shift pressed.");
    } else if (key == KEY_LCONTROL) {
        LOG_INFO("Left control pressed.");
    } else if (key == KEY_RCONTROL) {
        LOG_INFO("Right control pressed.");
    } else if (key == KEY_LALT) {
        LOG_INFO("Left alt pressed.");
    } else if (key == KEY_RALT) {
        LOG_INFO("Right alt pressed.");
    }

    if (state_ptr->keyboard_current.keys[key] != pressed) {
        state_ptr->keyboard_current.keys[key] = pressed;

        event_data event_data;
        event_data.u16[0] = key;
        trigger_event(pressed ? EVENT_KEY_PRESSED : EVENT_KEY_RELEASED, 0, event_data);
    }
}

void process_mouse_button(mouse_button button, b8 pressed) {
    if (state_ptr->mouse_current.buttons[button] != pressed) {
        state_ptr->mouse_current.buttons[button] = pressed;
        event_data event_data;
        event_data.u16[0] = button;
        trigger_event(pressed ? EVENT_MOUSE_BUTTON_PRESSED : EVENT_MOUSE_BUTTON_RELEASED, 0, event_data);
    }
}

void process_mouse_move(i16 x, i16 y) {
    if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {
        //LOG_DEBUG("Mouse pos: %i, %i", x, y);
        state_ptr->mouse_current.x = x;
        state_ptr->mouse_current.y = y;

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
    if (!state_ptr) {
        return FALSE;
    }
    return state_ptr->keyboard_current.keys[key] == FALSE;
}

b8 is_key_up(keys key) {
    if (!state_ptr) {
        return TRUE;
    }
    return state_ptr->keyboard_current.keys[key] == TRUE;
}

b8 was_key_down(keys key) {
    if (!state_ptr) {
        return FALSE;
    }
    return state_ptr->keyboard_previous.keys[key] == FALSE;
}

b8 was_key_up(keys key) {
    if (!state_ptr) {
        return TRUE;
    }
    return state_ptr->keyboard_previous.keys[key] == TRUE;
}

b8 is_mouse_button_down(mouse_button button) {
    if (!state_ptr) {
        return FALSE;
    }
    return state_ptr->mouse_current.buttons[button] == TRUE;
}

b8 is_mouse_button_up(mouse_button button) {
    if (!state_ptr) {
        return TRUE;
    }
    return state_ptr->mouse_current.buttons[button] == FALSE;
}

b8 was_mouse_button_down(mouse_button button) {
    if (!state_ptr) {
        return FALSE;
    }
    return state_ptr->mouse_previous.buttons[button] == TRUE;
}

b8 was_mouse_button_up(mouse_button button) {
    if (!state_ptr) {
        return TRUE;
    }
    return state_ptr->mouse_previous.buttons[button] == FALSE;
}

void get_mouse_position(i32 *x, i32 *y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_current.x;
    *y = state_ptr->mouse_current.y;
}

void get_previous_mouse_position(i32 *x, i32 *y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_previous.x;
    *y = state_ptr->mouse_previous.y;
}
