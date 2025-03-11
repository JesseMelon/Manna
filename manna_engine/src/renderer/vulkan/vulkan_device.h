#pragma once

#include "vulkan_types.h"

b8 create_vulkan_device(vulkan_context* context);

void destroy_vulkan_device(vulkan_context* context);

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface, vulkan_swapchain_support_info* out_support_info);
