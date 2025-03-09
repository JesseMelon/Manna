#include "mstring.h"
#include "core/memory.h"
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

