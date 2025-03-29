#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "core/logger.h"
#include "memory/memory.h"

b8 create_vulkan_buffer(vulkan_context *context, u64 size, VkBufferUsageFlagBits usage, u32 memory_property_flags, b8 bind_on_create, vulkan_buffer *out_buffer) {

    m_set_memory(out_buffer, 0, sizeof(vulkan_buffer)); //necessary?
    out_buffer->total_size = size;
    out_buffer->usage = usage;
    out_buffer->memory_property_flags = memory_property_flags;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //only used in one queue
    VkResult result = vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &out_buffer->handle);
    if (result) {
		LOG_ERROR("Failed to create vulkan buffer, %s", result);
    }

    //get memory requirements

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logical_device, out_buffer->handle, &requirements);
    out_buffer->memory_index = context->find_memory_index(requirements.memoryTypeBits, out_buffer->memory_property_flags);
    if (out_buffer->memory_index == -1) {
        LOG_ERROR("Unable to create vulkan buffer due to missing memory type index");
        return false;
    }

    VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = (u32) out_buffer->memory_index;

    result = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &out_buffer->memory);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Unable to create vulkan buffer, memory allocation failed. Error: %i", result);
        return false;
    }

    if (bind_on_create) {
        bind_vulkan_buffer(context, out_buffer, 0);
    }

    return true;
}

void destroy_vulkan_buffer(vulkan_context *context, vulkan_buffer *buffer) {
    if (buffer->memory) {
        vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
    buffer->total_size = 0;
    buffer->usage = 0;
    buffer->is_locked = false;
}

b8 resize_vulkan_buffer(vulkan_context* context, u64 new_size, vulkan_buffer* buffer, VkQueue queue, VkCommandPool pool) {
     
    //new buffer
    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = new_size;
    buffer_info.usage = buffer->usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //HACK: hardcoded to one queue sharing

    VkBuffer new_buffer;
    VK_CHECK(vkCreateBuffer(context->device.logical_device, &buffer_info, context->allocator, &new_buffer));

    //mem requirements
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logical_device, new_buffer, &requirements);

    //allocate info
    VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = (u32)buffer->memory_index;

    //allocate
    VkDeviceMemory new_memory;
    VkResult result = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &new_memory);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Unable to resize due to failed memory allocation. Error: %i", result);
        return false;
    }

    //bind new buffer
    VK_CHECK(vkBindBufferMemory(context->device.logical_device, new_buffer, new_memory, 0));

    //copy the data
    copy_vulkan_buffer(context, pool, 0, queue, buffer->handle, 0, new_buffer, 0, buffer->total_size);

    vkDeviceWaitIdle(context->device.logical_device);

    if (buffer->memory) {
        vkFreeMemory(context->device.logical_device, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(context->device.logical_device, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    //set new properties
    buffer->total_size = new_size;
    buffer->memory = new_memory;
    buffer->handle = new_buffer;

    return true;
}

void bind_vulkan_buffer(vulkan_context *context, vulkan_buffer *buffer, u64 offset) {
    VK_CHECK(vkBindBufferMemory(context->device.logical_device, buffer->handle, buffer->memory, offset));
}

void* lock_vulkan_buffer_memory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags) {
    void* data;
    VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data));
    return data;
}

void unlock_vulkan_buffer_memory(vulkan_context *context, vulkan_buffer *buffer) {
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void load_vulkan_buffer_data(vulkan_context *context, vulkan_buffer *buffer, u64 offset, u64 size, u32 flags, const void *data) {
    void* data_ptr;
    VK_CHECK(vkMapMemory(context->device.logical_device, buffer->memory, offset, size, flags, &data_ptr));
    m_copy_memory(data_ptr, data, size);
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void copy_vulkan_buffer(vulkan_context *context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer src, u64 src_offset, VkBuffer dest, u64 dest_offset, u64 size) {
    vkQueueWaitIdle(queue);
    vulkan_command_buffer temp_command_buffer;
    allocate_and_start_vulkan_command_buffer_one_off(context, pool, &temp_command_buffer);

    VkBufferCopy copy_region;
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dest_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(temp_command_buffer.handle, src, dest, 1, &copy_region);

    end_and_submit_vulkan_command_buffer_one_off(context, pool, &temp_command_buffer, queue);
}
