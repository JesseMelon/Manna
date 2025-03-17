#pragma once

#include "defines.h"

MANNA_API u64 get_string_length(const char* str);

MANNA_API char* duplicate_string(const char* str);

MANNA_API b8 compare_strings(const char* str0, const char* str1);

MANNA_API i32 format_string(char* dest, const char* format, ...);

MANNA_API i32 format_string_v(char* dest, const char* format, void* va_list);
