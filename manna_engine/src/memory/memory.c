#include "memory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "core/mstring.h"

//TODO not string.h
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>

struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_COUNT];
};

static const char* memory_tag_strings[MEMORY_TAG_COUNT] = {
	"UNKNOWN",
    "APPLICATION",
    "ARRAY",
    "LINEAR_ALOC",
    "DARRAY",
    "EDITOR",
    "STRING",
    "RENDERER"
};

typedef struct memory_state {
    struct memory_stats stats;
    u64 alloc_count;
} memory_state;

static memory_state* state_ptr;

void init_memory(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(memory_state);
    if (state == 0) {
        return;
    }
    state_ptr = state;
    state_ptr->alloc_count = 0;
	platform_set_memory(&state_ptr->stats, 0, sizeof(state_ptr->stats));
}
void shutdown_memory(void* state) {
    state_ptr = 0;
}

void* m_allocate(u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		LOG_WARN("allocating from unknown memory");
	}

    if(state_ptr) {
	    state_ptr->stats.total_allocated += size;
	    state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;
    }
	//TODO alignment
	void* memory = platform_allocate(size, FALSE);
	platform_set_memory(memory, 0, size);
	return memory;
}

void m_free(void* memory, u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		LOG_WARN("freeing from unknown memory tag");
	}
	
	state_ptr->stats.total_allocated -= size;
	state_ptr->stats.tagged_allocations[tag] -= size;

	//TODO alignment
	platform_free(memory, FALSE);
}

void* m_set_memory(void* memory, i32 value, u64 size) {
	return platform_set_memory(memory, value, size);
}

void* m_copy_memory(void* destination, const void* source, u64 size) {
	return platform_copy_memory(destination, source, size);
}
char* get_memory_usage() {
	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	char buffer[8000] = "System memory use: \n"; //TODO make const?
	u64 offset = strlen(buffer);

	for (u32 i = 0; i < MEMORY_TAG_COUNT; i++) {
		char unit[4] = "XiB";
		float amount = 1.0f;
		if (state_ptr->stats.tagged_allocations[i] >= gib) {
			unit[0] = 'G';
			amount = state_ptr->stats.tagged_allocations[i] / (float)gib;
		}
		else if (state_ptr->stats.tagged_allocations[i] >= mib) {
			unit[0] = 'M';
			amount = state_ptr->stats.tagged_allocations[i] / (float)mib;
		}
		else if (state_ptr->stats.tagged_allocations[i] >= kib) {
			unit[0] = 'K';
			amount = state_ptr->stats.tagged_allocations[i] / (float)kib;
		}
		else {
			unit[0] = 'B';
			unit[1] = 0;
			amount = (float)state_ptr->stats.tagged_allocations[i];
		}
		i32 len = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
		offset += len;
	}
	char* out_string = duplicate_string(buffer);
	return out_string; //free this
}

u64 get_memory_alloc_count() {
    if (state_ptr) {
        return state_ptr->alloc_count;
    } else {
        return 0;
    }
}
