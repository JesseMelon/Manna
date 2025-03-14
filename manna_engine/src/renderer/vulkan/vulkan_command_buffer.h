#pragma once
#include "vulkan_types.h"

//NOTE: command buffers can be allocated more than one at a time, this may have use.
/**
 * @brief [TODO:summary]
 *
 * @param[in] context [TODO:description]
 * @param[in] pool [TODO:description]
 * @param[in] is_primary A secondary command buffer cannot execute on its own, but within another command buffer
 * @param[out] out_command_buffer [TODO:description]
 */
void allocate_vulkan_command_buffer(vulkan_context* context, VkCommandPool pool, b8 is_primary, vulkan_command_buffer* out_command_buffer);

void free_vulkan_command_buffer(vulkan_context* context, VkCommandPool pool, vulkan_command_buffer* command_buffer);

void begin_vulkan_command_buffer(vulkan_command_buffer* command_buffer, b8 is_one_off, b8 continues_renderpass, b8 is_simultaneous);

void end_vulkan_command_buffer(vulkan_command_buffer* command_buffer);

void submit_vulkan_command_buffer(vulkan_command_buffer* command_buffer);

void reset_vulkan_command_buffer(vulkan_command_buffer* command_buffer);

void allocate_and_start_vulkan_command_buffer_one_off(vulkan_context* context, VkCommandPool pool, vulkan_command_buffer* out_command_buffer);

void end_and_submit_vulkan_command_buffer_one_off(vulkan_context* context, VkCommandPool pool, vulkan_command_buffer* command_buffer, VkQueue queue);

 
