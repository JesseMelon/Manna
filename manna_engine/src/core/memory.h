#pragma once

#include "defines.h"

typedef enum memory_tag {
	//TODO add tags
	MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_DARRAY,
	MEMORY_TAG_COUNT
} memory_tag;

MANNA_API void init_memory();
MANNA_API void shutdown_memory();

MANNA_API void* m_allocate(u64 size, memory_tag tag);
MANNA_API void m_free(void* ptr, u64 size, memory_tag tag);
MANNA_API void* m_set_memory(void* ptr, i32 value, u64 size);
MANNA_API void* m_copy_memory(void* dest, const void* src, u64 size);
MANNA_API char* get_memory_usage();
