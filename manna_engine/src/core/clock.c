#include "clock.h"
#include "platform/platform.h"

void update_clock(clock *clock) {
    if (clock->start_time != 0) {
        clock->elapsed = platform_get_time() - clock->start_time;
    }
}

void start_clock(clock *clock) {
    clock->start_time = platform_get_time();
    clock->elapsed = 0;
}

void stop_clock(clock *clock) {
    clock->start_time = 0;
}
