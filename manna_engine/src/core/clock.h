#pragma once
#include "defines.h"

typedef struct clock {
    f64 start_time;
    f64 elapsed;
} clock;

MANNA_API void update_clock(clock* clock);

MANNA_API void start_clock(clock* clock);

MANNA_API void stop_clock(clock* clock);
