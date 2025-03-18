#pragma once

#include "defines.h"
#include "core/asserts.h"
#include <vulkan/vulkan.h>
#include "vulkan/vulkan_core.h"

#define VK_CHECK(expression)                            \
    do {                                                \
        VkResult result = expression;                   \
        M_ASSERT(result == VK_SUCCESS, "");             \
        (void)result;                                   \
    } while(0)

//system swapchain capabilities
typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

//overall system capabilities
typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;

    //indices of queues
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkCommandPool graphics_command_pool;

    //handles to queues themselves
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;
} vulkan_device;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_renderpass_state {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_renderpass_state;

typedef struct vulkan_renderpass {
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;
    vulkan_renderpass_state state;
} vulkan_renderpass;

typedef struct vulkan_framebuffer {
    VkFramebuffer handle;
    u32 attachment_count;
    VkImageView* attachments;
    vulkan_renderpass* renderpass;
} vulkan_framebuffer;

typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;
    vulkan_image depth_attachment;
    vulkan_framebuffer* framebuffers;
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_READY,
    COMMAND_BUFFER_RECORDING,
    COMMAND_BUFFER_IN_RENDER_PASS,
    COMMAND_BUFFER_RECORDING_ENDED,
    COMMAND_BUFFER_SUBMITTED,
    COMMAND_BUFFER_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct vulkan_fence {
    VkFence handle;
    b8 is_signaled;
} vulkan_fence;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
} vulkan_pipeline;

#define OBJECT_SHADER_STAGE_COUNT 2 //vert & frag

typedef struct vulkan_object_shader {
    vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT];
    vulkan_pipeline pipeline;
} vulkan_object_shader;

typedef struct vulkan_context {
    u32 framebuffer_width;
    u32 framebuffer_height;
    u64 framebuffer_size_generation;
    u64 framebuffer_size_last_generation;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
#if defined (DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    vulkan_device device;
    vulkan_swapchain swapchain;
    vulkan_renderpass main_renderpass;

    //darray
    vulkan_command_buffer* graphics_command_buffers;

    //darray
    VkSemaphore* image_available_semaphores;

    //darray
    VkSemaphore* queue_complete_semaphores;

    u32 in_flight_fence_count;
    vulkan_fence* in_flight_fences;

    vulkan_fence** images_in_flight;

    u32 image_index;
    u32 current_frame;
    b8 recreating_swapchain;

    vulkan_object_shader object_shader;

    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
} vulkan_context;

