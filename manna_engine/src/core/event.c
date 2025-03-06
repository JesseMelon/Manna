#include "core/event.h"
#include "core/memory.h"

 typedef struct event_listener {
    void* listener;
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
