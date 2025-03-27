#pragma once

#include "vulkan_types.h"

b8 create_vulkan_buffer(vulkan_context* context, u64 size, VkBufferUsageFlagBits usage, u32 memory_property_flags, b8 bind_on_create, vulkan_buffer* out_buffer);

void destroy_vulkan_buffer(vulkan_context* context, vulkan_buffer* buffer);

b8 resize_vulkan_buffer(vulkan_context* context, u64 new_size, vulkan_buffer* buffer, VkQueue queue, VkCommandPool pool);

void bind_vulkan_buffer(vulkan_context* context, vulkan_buffer* buffer, u64 offset);

void* lock_vulkan_buffer_memory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags);
void unlock_vulkan_buffer_memory(vulkan_context* context, vulkan_buffer* buffer);

void load_vulkan_buffer_data(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags, const void* data);

void copy_vulkan_buffer(vulkan_context* context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer src, u64 src_offset, VkBuffer dest, u64 dest_offset, u64 size);
