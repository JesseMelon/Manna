#include "vulkan_fence.h"
#include "core/logger.h"

void create_vulkan_fence(vulkan_context *context, b8 start_signaled, vulkan_fence *out_fence) {
    out_fence->is_signaled = start_signaled;
    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (out_fence->is_signaled) {
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(context->device.logical_device, &fence_create_info, context->allocator, &out_fence->handle));
}

void destroy_vulkan_fence(vulkan_context* context, vulkan_fence* fence) {
    if (fence->handle) {
        vkDestroyFence(context->device.logical_device, fence->handle, context->allocator);
        fence->handle = 0;
    }
    fence->is_signaled = FALSE;
}

b8 wait_for_vulkan_fence(vulkan_context *context, vulkan_fence *fence, u64 timeout_ns) {
    if (!fence->is_signaled) {
        VkResult result = vkWaitForFences(context->device.logical_device, 1, &fence->handle, TRUE, timeout_ns);
        switch (result) {
            case VK_SUCCESS:
                fence->is_signaled = TRUE;
                return TRUE;
            case VK_TIMEOUT:
                LOG_WARN("Wait for fence timed out.");
                break;
            case VK_ERROR_DEVICE_LOST:
                LOG_ERROR("Wait for fence - device lost.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                LOG_ERROR("Wait for fence - out of host memory.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                LOG_ERROR("Wait for fence - out of device memory.");
                break;
            default:
                LOG_ERROR("Wait for fence - unknown error.");
                break;
        }
    } else {
        return TRUE;
    }
    return FALSE;
}

void reset_vulkan_fence(vulkan_context *context, vulkan_fence *fence) {
    if (fence->is_signaled) {
        VK_CHECK(vkResetFences(context->device.logical_device, 1, &fence->handle));
        fence->is_signaled = FALSE;
    }
}
