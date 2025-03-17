#include "vulkan_command_buffer.h"
#include "memory/memory.h"

void allocate_vulkan_command_buffer(vulkan_context *context, VkCommandPool pool, b8 is_primary, vulkan_command_buffer *out_command_buffer) {
    VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};

    m_set_memory(out_command_buffer, 0, sizeof(vulkan_command_buffer));

    allocate_info.commandPool = pool;
    allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = 0;
    
    out_command_buffer->state = COMMAND_BUFFER_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(context->device.logical_device, &allocate_info, &out_command_buffer->handle));
    out_command_buffer->state = COMMAND_BUFFER_READY;
}

void free_vulkan_command_buffer(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *command_buffer) {
    vkFreeCommandBuffers(context->device.logical_device, pool, 1, &command_buffer->handle);
    command_buffer->handle = 0;
    command_buffer->state = COMMAND_BUFFER_NOT_ALLOCATED;
}

void begin_vulkan_command_buffer(vulkan_command_buffer *command_buffer, b8 is_one_time, b8 continues_renderpass, b8 is_simultaneous) {
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = 0;
    if (is_one_time) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (continues_renderpass) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (is_simultaneous) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->state = COMMAND_BUFFER_RECORDING;
}

//TODO: inline a bunch of these
void end_vulkan_command_buffer(vulkan_command_buffer* command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_RECORDING_ENDED;
}

void submit_vulkan_command_buffer(vulkan_command_buffer *command_buffer) {
    command_buffer->state = COMMAND_BUFFER_SUBMITTED;
}

void reset_vulkan_command_buffer(vulkan_command_buffer *command_buffer) {
    command_buffer->state = COMMAND_BUFFER_READY;
}

void allocate_and_start_vulkan_command_buffer_one_off(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *out_command_buffer) {
    allocate_vulkan_command_buffer(context, pool, TRUE, out_command_buffer);
    begin_vulkan_command_buffer(out_command_buffer, TRUE, FALSE, FALSE);
}

void end_and_submit_vulkan_command_buffer_one_off(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *command_buffer, VkQueue queue) {
    end_vulkan_command_buffer(command_buffer);
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

    //wait for it to finish
    VK_CHECK(vkQueueWaitIdle(queue));

    free_vulkan_command_buffer(context, pool, command_buffer);
}
