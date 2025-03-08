#include "core/event.h"
#include "containers/darray.h"
#include "core/memory.h"
#include "defines.h"

//TODO: Jobify, defer

typedef struct event_listener {
    void* instance;
    event_handler on_event;
} event_listener;

typedef struct event {
    event_listener* listeners;
}event;

#define MAX_EVENTS 16384

typedef struct event_system_state {
    event events[MAX_EVENTS];
}event_system_state;

static b8 initialized = FALSE;
static event_system_state state;

b8 init_events() {
    if (initialized == TRUE) {
        return FALSE;
    }
    m_set_memory(&state, 0, sizeof(state));
    initialized = TRUE;
    return TRUE;
}

void shutdown_events() {
    for (u16 i = 0; i < MAX_EVENTS; ++i) {
        if (state.events[i].listeners != 0) {
            darray_destroy(state.events[i].listeners);
            state.events[i].listeners = 0; //nullptr
        }
    }
}

b8 listen_to_event(u16 event_id, void *listener, event_handler on_event) {
    //these ifs are kindof expensive. Should probably remove from dist and/or release builds
    if (!initialized) {
        return FALSE;
    }
    if (state.events[event_id].listeners == 0) {
        state.events[event_id].listeners = darray_create(event_listener);
    }
    u64 listener_count = darray_get_length(state.events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        if (state.events[event_id].listeners->instance == listener) {
            //TODO: warn
            return FALSE;
        }
    }
    event_listener event_listener;
    event_listener.instance = listener;
    event_listener.on_event = on_event;
    darray_push(state.events[event_id].listeners, event_listener);
    
    return TRUE;
}

b8 ignore_event(u16 event_id, void *listener, event_handler on_event) {
    if (initialized == FALSE) {
        return FALSE;
    }
    if (state.events[event_id].listeners == 0) {
        //TODO: warn
        return FALSE;
    }
    u64 listener_count = darray_get_length(state.events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        event_listener event_listener = state.events[event_id].listeners[i];
        if (event_listener.instance == listener && event_listener.on_event == on_event) {
            struct event_listener popped_listener;
            darray_pop_at(state.events[event_id].listeners, i, &popped_listener);
            return TRUE;
        }
    }
    //not found, should probably warn here aswell.
    return FALSE;
}

b8 trigger_event(u16 event_id, void *sender, event_data event_data) {
    if (initialized == FALSE) {
        return FALSE;
    }
    if (state.events[event_id].listeners == 0) {
        //TODO: warn
        return FALSE;
    }
    u64 listener_count = darray_get_length(state.events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        event_listener event_listener = state.events[event_id].listeners[i];
        if (event_listener.on_event(event_id, sender, event_listener.instance, event_data)) { //on_event function returns a bool for the if.
            //handled
            return TRUE;
        }
    }
    //not found. warn?
    return FALSE;
}
