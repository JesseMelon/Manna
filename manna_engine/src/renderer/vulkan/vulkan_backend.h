#pragma once

#include "renderer/renderer_types.h"

/**
 * @brief Initializes the Vulkan renderer backend
 *
 * this function sets up the Vulkan rendering system by creating a Vulkan instance, configuring required
 * extensions and validation layers (in debug mode), establishing surface creation and initializing the Vulkan device.
 * It also sets up a debug messenger for logging vulkan errors and warnings while debugging.
 *
 * @param[in,out] backend pointer to the backend structure being initialized
 * @param[in] application_name null terminated string specifying name of application used in the vulkan instance
 * @param[in] platform_state platform state for platform specific surface creation
 * @return TRUE if initialization succeeds. FALSE otherwise.
 */
b8 init_vulkan_renderer_backend(renderer_backend* backend, const char* application_name);

/**
 * @brief cleans up everything created by init_vulkan_renderer_backend
 *
 * @param[in,out] backend pointer to backend structure for shutdown
 */
void shutdown_vulkan_renderer_backend(renderer_backend* backend);

void vulkan_renderer_backend_on_resize(renderer_backend* backend, u16 width, u16 height);

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time);

void vulkan_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time);

void vulkan_backend_update_object(mat4 model);
