#include "core/logger.h"
#include "defines.h"
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_shader_utils.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 create_vulkan_object_shader(vulkan_context *context, vulkan_object_shader *out_shader) {
    char stage_type_strings[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strings[i], stage_types[i], i, out_shader->stages)) {
            LOG_ERROR("Unable to create %s shader module for '%s'", stage_type_strings[i], BUILTIN_SHADER_NAME_OBJECT);
            return FALSE;
        }   
    }
    
    //descriptors
    return true;
}

void destroy_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {

}

void use_vulkan_object_shader(vulkan_context *context, struct vulkan_object_shader *shader) {

}
