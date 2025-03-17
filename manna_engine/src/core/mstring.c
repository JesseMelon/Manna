#include "mstring.h"
#include "memory/memory.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

u64 get_string_length(const char* str) {
    return strlen(str);
}

char* duplicate_string(const char* str) {
    u64 length = get_string_length(str);
    char* copy = m_allocate(length + 1, MEMORY_TAG_STRING);
    m_copy_memory(copy, str, length + 1);
    return copy;
}

b8 compare_strings(const char *str0, const char *str1) {
    return strcmp(str0, str1) == 0;
}

i32 format_string(char* dest, const char *format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr; //remove __builtin if VS gives trouble
        va_start(arg_ptr, format);
        i32 written = format_string_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 format_string_v(char *dest, const char *format, void *va_listp) {
    if (dest) {
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        m_copy_memory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}
