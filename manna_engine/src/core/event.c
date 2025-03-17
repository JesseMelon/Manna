#include "core/event.h"
#include "containers/darray.h"
#include "memory/memory.h"
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

static event_system_state* state_ptr;

void init_events(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(event_system_state);
    if (state == 0) {
        return;
    }
    m_set_memory(state, 0, sizeof(state));
    state_ptr = state;
    return;
}

void shutdown_events(void* state) {
    if (state_ptr) {
        for (u16 i = 0; i < MAX_EVENTS; ++i) {
            if (state_ptr->events[i].listeners != 0) {
                darray_destroy(state_ptr->events[i].listeners);
                state_ptr->events[i].listeners = 0; //nullptr
            }
        }
    }
    state_ptr = 0;
}

b8 listen_to_event(u16 event_id, void *listener, event_handler on_event) {
    //these ifs are kindof expensive. Should probably remove from dist and/or release builds
    if (!state_ptr) {
        return FALSE;
    }
    if (state_ptr->events[event_id].listeners == 0) {
        state_ptr->events[event_id].listeners = darray_create(event_listener);
    }
    u64 listener_count = darray_get_length(state_ptr->events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        if (state_ptr->events[event_id].listeners->instance == listener) {
            //TODO: warn
            return FALSE;
        }
    }
    event_listener event_listener;
    event_listener.instance = listener;
    event_listener.on_event = on_event;
    darray_push(state_ptr->events[event_id].listeners, event_listener);
    
    return TRUE;
}

b8 ignore_event(u16 event_id, void *listener, event_handler on_event) {
    if (!state_ptr) {
        return FALSE;
    }
    if (state_ptr->events[event_id].listeners == 0) {
        //TODO: warn
        return FALSE;
    }
    u64 listener_count = darray_get_length(state_ptr->events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        event_listener event_listener = state_ptr->events[event_id].listeners[i];
        if (event_listener.instance == listener && event_listener.on_event == on_event) {
            struct event_listener popped_listener;
            darray_pop_at(state_ptr->events[event_id].listeners, i, &popped_listener);
            return TRUE;
        }
    }
    //not found, should probably warn here aswell.
    return FALSE;
}

b8 trigger_event(u16 event_id, void *sender, event_data event_data) {
    if (!state_ptr) {
        return FALSE;
    }
    if (state_ptr->events[event_id].listeners == 0) {
        //TODO: warn
        return FALSE;
    }
    u64 listener_count = darray_get_length(state_ptr->events[event_id].listeners);
    for (u64 i = 0; i < listener_count; ++i) {
        event_listener event_listener = state_ptr->events[event_id].listeners[i];
        if (event_listener.on_event(event_id, sender, event_listener.instance, event_data)) { //on_event function returns a bool for the if.
            //handled
            return TRUE;
        }
    }
    //not found. warn?
    return FALSE;
}
