#pragma once
#include "defines.h"

typedef struct clock {
    f64 start_time;
    f64 elapsed;
} clock;

void update_clock(clock* clock);

void start_clock(clock* clock);

void stop_clock(clock* clock);
