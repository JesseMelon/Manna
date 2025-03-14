#pragma once

#include "vulkan_types.h"

void create_vulkan_framebuffer(vulkan_context* context, vulkan_renderpass* renderpass, u32 width, u32 height, u32 attachment_count, VkImageView* attachments, vulkan_framebuffer* out_framebuffer);

void destroy_vulkan_framebuffer(vulkan_context* context, vulkan_framebuffer* framebuffer);
