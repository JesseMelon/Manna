#pragma once

#include "vulkan_types.h"

void create_vulkan_image(
    vulkan_context* context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image);

void create_vulkan_image_view(vulkan_context* context, VkFormat format, vulkan_image* image, VkImageAspectFlags aspect_flags);

void destroy_vulkan_image(vulkan_context* context, vulkan_image* image);


