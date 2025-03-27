#include "vulkan_shader_utils.h"

#include "core/mstring.h"
#include "core/logger.h"
#include "memory/memory.h"
#include "platform/filesystem.h"

b8 create_shader_module(vulkan_context *context, const char *name, const char *type_str, VkShaderStageFlagBits shader_stage_flag, u32 stage_index, vulkan_shader_stage *shader_stages) {
    //build file name
    char file_name[512];
    //TODO: configurable path
    format_string(file_name, "../../shaders/%s.%s.spv", name, type_str);

    m_set_memory(&shader_stages[stage_index].create_info, 0, sizeof(VkShaderModuleCreateInfo));
    shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    
    file_handle handle;
    if (!open_file(file_name, FILE_MODE_READ, TRUE, &handle)) {
        LOG_ERROR("Unable to read shader module: %s.", file_name);
        return false;
    }

    //read as binary
    u64 size = 0;
    u8* file_buffer = 0;
    if (!read_all_bytes(&handle, &file_buffer, &size)) {
        LOG_ERROR("Unable to read binary shader module: '%s'", file_name);
        return false;
    }
    shader_stages[stage_index].create_info.codeSize = size;
    shader_stages[stage_index].create_info.pCode = (u32*)file_buffer;

    close_file(&handle);

    VK_CHECK(vkCreateShaderModule(context->device.logical_device, &shader_stages[stage_index].create_info, context->allocator, &shader_stages[stage_index].handle));

    //shader stage info
    m_set_memory(&shader_stages[stage_index].shader_stage_create_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
    shader_stages[stage_index].shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[stage_index].shader_stage_create_info.stage = shader_stage_flag;
    shader_stages[stage_index].shader_stage_create_info.module = shader_stages[stage_index].handle;
    //NOTE: CAN HAVE MULTIPLE ENTRY POINTS ON SAME SHADER FILE. USEFUL.
    shader_stages[stage_index].shader_stage_create_info.pName = "main";

    if (file_buffer) {
        m_free(file_buffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        file_buffer = 0;
    }
    return true;
}
