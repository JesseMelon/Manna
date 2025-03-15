#pragma once
#include "vulkan_types.h"

/**
 * @brief returns the string representation of a result
 *
 * @param[in] result the result returned by a vulkan call
 * @param[in] get_extended whether to also include extended information
 * @return the error code and extended error info as string
 */
const char* vulkan_result_string(VkResult result, b8 get_extended);

/**
 * @brief indicates whether result is success or error
 *
 * @param[in] result the result returned by a vulkan call
 * @return TRUE if success
 */
b8 vulkan_result_is_success(VkResult result);
