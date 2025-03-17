#include "linear_allocator.h"
#include "memory/memory.h"
#include "core/logger.h"

void create_linear_allocator(u64 total_size, void *memory, linear_allocator *out_allocator) {
    if(out_allocator) {
        out_allocator->total_size = total_size;
        out_allocator->allocated = 0;
        out_allocator->owns_memory = memory == 0;
        if (memory) {
            out_allocator->memory = memory;
        } else {
            out_allocator->memory = m_allocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void destroy_linear_allocator(linear_allocator *allocator) {
    if (allocator) {
        allocator->total_size = 0;
        allocator->allocated = 0;
        if (allocator->owns_memory && allocator->memory) {
            m_free(allocator->memory, allocator->total_size, MEMORY_TAG_LINEAR_ALLOCATOR);       
        } else {
            allocator->memory = 0;
        }
        allocator->total_size = 0;
        allocator->owns_memory = FALSE;
    }
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size) {
    if (allocator && allocator -> memory) {
        if (allocator->allocated + size > allocator->total_size) {
            u64 remaining = allocator->total_size - allocator->allocated;
            LOG_ERROR("Linear allocator full %lluB, only %lluB remaining", size, remaining);
            return 0;
        }

        void* block = allocator->memory + allocator->allocated;
        allocator->allocated += size;
        return block;
    } else {
        LOG_ERROR("Linear allocator not intitialized");
        return 0;
    }
}

void linear_allocator_free_all(linear_allocator *allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        m_set_memory(allocator->memory, 0, allocator->total_size);
    }
}
