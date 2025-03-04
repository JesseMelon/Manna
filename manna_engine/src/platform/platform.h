#pragma once

#include "defines.h"

typedef struct platform_state {
	void* internal_state;
} platform_state;

b8 platform_startup(
	platform_state* platform_state,
	const char* title,
	i32 x,
	i32 y,
	i32 width,
	i32 height);

//TODO: remove API exports
void platform_shutdown(platform_state* platform_state);

b8 platform_get_messages(platform_state* platform_state);

MANNA_API void* platform_allocate(u64, b8 aligned);
MANNA_API void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dst, void* src, u64 size);
void* platform_set_memory(void* block, u8 value, u64 size);

void platform_console_write(const char* str, u8 color);
void platform_console_write_error(const char* str, u8 color); //distinguished error channel to stream elsewhere if desired

f64 platform_get_time();

void platform_sleep(u64 ms);
