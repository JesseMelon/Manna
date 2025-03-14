#pragma once

#include "vulkan_types.h"

/**
 * @brief Creates the vulkan renderpass, which may consist of subpasses.
 *
 * @param[in] context the global rendering state
 * @param[out] out_renderpass the handle to the created object
 * @param[in] x x position of surface
 * @param[in] y y position of surface
 * @param[in] w width of rendered image
 * @param[in] h height of rendererd image
 * @param[in] r clear color red value
 * @param[in] g green
 * @param[in] b blue
 * @param[in] a alpha
 * @param[in] depth [TODO:description]
 * @param[[TODO:direction]] stencil [TODO:description]
 */
void create_vulkan_renderpass(vulkan_context* context, vulkan_renderpass* out_renderpass, f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a, f32 depth, u32 stencil);

void destroy_vulkan_renderpass(vulkan_context* context, vulkan_renderpass* renderpass);

void vulkan_renderpass_begin(vulkan_command_buffer* command_buffer, vulkan_renderpass* renderpass, VkFramebuffer frame_buffer);

void vulkan_renderpass_end(vulkan_command_buffer* command_buffer, vulkan_renderpass* renderpass); 
