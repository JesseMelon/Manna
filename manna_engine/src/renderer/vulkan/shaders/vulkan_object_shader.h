#pragma once

#include "renderer/vulkan/vulkan_types.h"

b8 create_vulkan_object_shader(vulkan_context* context, vulkan_object_shader* out_shader);

void destroy_vulkan_object_shader(vulkan_context* context, struct vulkan_object_shader* shader);

//pass shader as an arg rather than its own func?
void use_vulkan_object_shader(vulkan_context* context, struct vulkan_object_shader* shader);

void object_shader_update_global_ubo(vulkan_context* context, struct vulkan_object_shader* shader);

void object_shader_update_object(vulkan_context* context, mat4 model);
