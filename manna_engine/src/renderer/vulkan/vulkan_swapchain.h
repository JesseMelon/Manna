#pragma once

#include "vulkan_types.h"

void create_vulkan_swapchain(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* out_swapchain);

void recreate_vulkan_swapchain(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* out_swapchain);

void destroy_vulkan_swapchain(vulkan_context* context, vulkan_swapchain* swapchain);

b8 vulkan_swapchain_acquire_next_image_index(vulkan_context* context, vulkan_swapchain* swapchain, u64 timeout_ns, VkSemaphore image_available_sem, VkFence fence, u32* out_image_index);

void vulkan_swapchain_present_image(vulkan_context* context, vulkan_swapchain* swapchain, VkQueue graphics_queue, VkQueue present_queue, VkSemaphore render_complete_sem, u32 present_image_index);
