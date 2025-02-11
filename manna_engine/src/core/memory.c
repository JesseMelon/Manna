#include "memory.h"
#include "core/logger.h"
#include "platform/platform.h"

//TODO not string.h
#include <string.h>
#include <stdio.h>

struct memory_stats {
	u64 total_allocated;
	u64 tagged_allocations[MEMORY_TAG_COUNT];
};

static const char* memory_tag_strings[MEMORY_TAG_COUNT] = {
	"UNKNOWN",
};

static struct memory_stats stats;

void initialize_memory() {
	platform_set_memory(&stats, 0, sizeof(stats));
}
void shutdown_memory() {}

void* m_allocate(u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		M_WARN("allocating from unknown memory");
	}

	stats.total_allocated += size;
	stats.tagged_allocations[tag] += size;

	//TODO alignment
	void* memory = platform_allocate(size, FALSE);
	platform_set_memory(memory, 0, size);
	return memory;
}

void m_free(void* memory, u64 size, memory_tag tag) {
	if (tag == MEMORY_TAG_UNKNOWN) {
		M_WARN("freeing from unknown memory tag");
	}
	
	stats.total_allocated -= size;
	stats.tagged_allocations[tag] -= size;

	//TODO alignment
	platform_free(memory, FALSE);
}

void* m_set_memory(void* memory, i32 value, u64 size) {
	return platform_set_memory(memory, value, size);
}

void* m_copy_memory(void* destination, void* source, u64 size) {
	return platform_copy_memory(destination, source, size);
}
char* get_memory_usage() {
	const u64 gib = 1024 * 1024 * 1024;
	const u64 mib = 1024 * 1024;
	const u64 kib = 1024;

	char buffer[8000] = "System memory use: \n";
	u64 offset = strlen(buffer);

	for (u32 i = 0; i < MEMORY_TAG_COUNT; i++) {
		char unit[4] = "XiB";
		float amount = 1.0f;
		if (stats.tagged_allocations[i] >= gib) {
			unit[0] = 'G';
			amount = stats.tagged_allocations[i] / (float)gib;
		}
		else if (stats.tagged_allocations[i] >= mib) {
			unit[0] = 'M';
			amount = stats.tagged_allocations[i] / (float)mib;
		}
		else if (stats.tagged_allocations[i] >= kib) {
			unit[0] = 'K';
			amount = stats.tagged_allocations[i] / (float)kib;
		}
		else {
			unit[0] = 'B';
			unit[1] = '0';
			amount = (float)stats.tagged_allocations[i];
		}
		i32 len = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
		offset += len;
	}
	char* out_string = _strdup(buffer);
	return out_string; //free this
}