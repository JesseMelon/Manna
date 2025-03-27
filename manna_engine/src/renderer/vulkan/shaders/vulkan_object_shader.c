#include "core/logger.h"
#include "defines.h"
#include "math/math_types.h"
#include "memory/memory.h"
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 create_vulkan_object_shader(vulkan_context *context, vulkan_object_shader *out_shader) {

    //init shader module stages
    char stage_type_strings[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strings[i], stage_types[i], i, out_shader->stages)) {
            LOG_ERROR("Unable to create %s shader module for '%s'", stage_type_strings[i], BUILTIN_SHADER_NAME_OBJECT);
            return FALSE;
        }   
    }
    
    //descriptors

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebuffer_height;
    viewport.width = (f32)context->framebuffer_width;
    viewport.height = -(f32)context->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebuffer_width;
    scissor.extent.height = context->framebuffer_height;

    //attributes
    u32 offset = 0;
#define ATTRIBUTE_COUNT 1
    VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];
    //position
    VkFormat formats[ATTRIBUTE_COUNT] = {
        VK_FORMAT_R32G32B32_SFLOAT //32 bit floats for x, y, and z
    };
    //sizes
    u64 sizes[ATTRIBUTE_COUNT] = {
        sizeof(vec3)
    };
    for (u32 i = 0; i < ATTRIBUTE_COUNT; ++i) {
        attribute_descriptions[i].binding = 0; //binding index, should match binding description
        attribute_descriptions[i].location = i; //attribute location. maps to layout(location) in shader
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    //TODO: descriptor set layouts

    //stages
    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    m_set_memory(stage_create_infos, 0, sizeof(stage_create_infos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
    }

    if (!create_vulkan_pipeline(
        context,
        &context->main_renderpass,
        ATTRIBUTE_COUNT,
        attribute_descriptions,
        0,
        0,
        OBJECT_SHADER_STAGE_COUNT,
        stage_create_infos,
        viewport,
        scissor,
        false,
        &out_shader->pipeline)) {
        LOG_ERROR("Failed to load graphics pipeline for object shader");
        return false;
    }

    return true;
}

void destroy_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {
    
    destroy_vulkan_pipeline(context, &shader->pipeline);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logical_device, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void use_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {
    u32 image_index = context->image_index;
    bind_vulkan_pipeline(&context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}
