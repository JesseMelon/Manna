#pragma once

#include "vulkan_types.h"

void create_vulkan_fence(vulkan_context* context, b8 start_signaled, vulkan_fence* out_fence);

void destroy_vulkan_fence(vulkan_context* context, vulkan_fence* fence);

/**
 * @brief blocking
 *
 * @param[in] context [TODO:description]
 * @param[in] fence [TODO:description]
 * @param[in] timeout_ns [TODO:description]
 * @return [TODO:description]
 */
b8 wait_for_vulkan_fence(vulkan_context* context, vulkan_fence* fence, u64 timeout_ns);

void reset_vulkan_fence(vulkan_context* context,vulkan_fence* fence);
