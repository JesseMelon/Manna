#pragma once

#include "defines.h"

typedef struct linear_allocator {
    u64 total_size;
    u64 allocated;
    void* memory;
    //if we must call malloc or free, or if this is allocating from another allocator
    b8 owns_memory;
} linear_allocator;

MANNA_API void create_linear_allocator(u64 total_size, void* memory, linear_allocator* out_allocator);
MANNA_API void destroy_linear_allocator(linear_allocator* allocator);

MANNA_API void* linear_allocator_allocate(linear_allocator* allocator, u64 size);
MANNA_API void linear_allocator_free_all(linear_allocator* allocator);
