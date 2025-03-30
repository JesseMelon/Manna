#include "core/logger.h"
#include "defines.h"
#include "math/math_types.h"
#include "memory/memory.h"
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_buffer.h"

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

    //global descriptors
    VkDescriptorSetLayoutBinding global_ubo_layout_binding;
    global_ubo_layout_binding.binding = 0;
    global_ubo_layout_binding.descriptorCount = 1;
    global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_ubo_layout_binding.pImmutableSamplers = 0;
    global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //only use in vertex stage

    VkDescriptorSetLayoutCreateInfo global_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    global_layout_info.bindingCount = 1;
    global_layout_info.pBindings = &global_ubo_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device, &global_layout_info, context->allocator, &out_shader->global_descriptor_set_layout));

    VkDescriptorPoolSize global_pool_size;
    global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_pool_size.descriptorCount = context->swapchain.image_count;

    VkDescriptorPoolCreateInfo global_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    global_pool_info.poolSizeCount = 1;
    global_pool_info.pPoolSizes = &global_pool_size;
    global_pool_info.maxSets = context->swapchain.image_count; //how many actual sets in this pool
    VK_CHECK(vkCreateDescriptorPool(context->device.logical_device, &global_pool_info, context->allocator, &out_shader->global_descriptor_pool));

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

    //descriptor set layouts
#define DESCRIPTOR_SET_LAYOUT_COUNT 1
    VkDescriptorSetLayout layouts[DESCRIPTOR_SET_LAYOUT_COUNT] = {
        out_shader->global_descriptor_set_layout
    };
    
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
                DESCRIPTOR_SET_LAYOUT_COUNT,
                layouts,
                OBJECT_SHADER_STAGE_COUNT,
                stage_create_infos,
                viewport,
                scissor,
                false,
                &out_shader->pipeline)) {
        LOG_ERROR("Failed to load graphics pipeline for object shader");
        return false;
    }

    //create the uniform buffer
    if (!create_vulkan_buffer(context, sizeof(global_uniform_object) * 3,
                              //a transfer destination aswell as uniform buffer
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                              //device local, and visible/coherent to be uploaded directly
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              //bound on creation
                              true, &out_shader->global_ubo)) {
        LOG_ERROR("Vulkan buffer creation failed in object shader");
        return false;
    }

    //provide layout per descriptor set
    VkDescriptorSetLayout global_layouts[3] = {
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout
    };

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = out_shader->global_descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device, &alloc_info, out_shader->global_descriptor_sets));
    
    return true;
}

void destroy_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {
    
    destroy_vulkan_buffer(context, &shader->global_ubo);

    destroy_vulkan_pipeline(context, &shader->pipeline);

    vkDestroyDescriptorPool(context->device.logical_device, shader->global_descriptor_pool, context->allocator);

    vkDestroyDescriptorSetLayout(context->device.logical_device, shader->global_descriptor_set_layout, context->allocator);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logical_device, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void use_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {
    u32 image_index = context->image_index;
    bind_vulkan_pipeline(&context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void vulkan_object_shader_update_global_ubo(vulkan_context *context, struct vulkan_object_shader *shader) {
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = shader->global_descriptor_sets[image_index];

    if (!shader->descriptor_updated[image_index]) {
        u32 range = sizeof(global_uniform_object);
        u64 offset = sizeof(global_uniform_object) * image_index;

        //copy to buffer
        load_vulkan_buffer_data(context, &shader->global_ubo, offset, range, 0, &shader->global_uniform_object);

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = shader->global_ubo.handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor_write.dstSet = shader->global_descriptor_sets[image_index];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(context->device.logical_device, 1, &descriptor_write, 0, 0);
        //shader->descriptor_updated[image_index] = true;
    }

    //bind the global descriptor set to be updated
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 0, 1, &global_descriptor, 0, 0);
}

//vulkan spec garantees 128 byte push constant
void vulkan_object_shader_update_object(vulkan_context *context, struct vulkan_object_shader* shader, mat4 model) {
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;

    vkCmdPushConstants(command_buffer, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);
}
