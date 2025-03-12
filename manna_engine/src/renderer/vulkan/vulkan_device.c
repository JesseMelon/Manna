#include "vulkan_device.h"
#include "core/logger.h"
#include "core/mstring.h"
#include "core/memory.h"
#include "containers/darray.h"
#include "defines.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
#include <strings.h>

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    //darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulcan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface, vulkan_swapchain_support_info *out_support_info) {
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &out_support_info->capabilities));
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, 0));
    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = m_allocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, out_support_info->formats));
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, 0));
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = m_allocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, out_support_info->present_modes));
    }
}

b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support) {

    //determine if device meets needs of application
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            LOG_INFO("Device is not a discrete GPU as required. Skipping.");
            return FALSE;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    //see what each queue supports
    LOG_INFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;
        }
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            ++current_transfer_score;
        }
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            //take the index if it is current lowest, landing on designated transfer queue by the end
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }
        
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if (supports_present) {
            out_queue_info->present_family_index = i;
        }
    }
    LOG_INFO("       %d |       %d |       %d |        %d | %s ",
        out_queue_info->graphics_family_index != -1,
        out_queue_info->present_family_index != -1,
        out_queue_info->compute_family_index != -1,
        out_queue_info->transfer_family_index != -1,
        properties->deviceName);
    if (
        (!requirements->graphics || out_queue_info->graphics_family_index != -1) &&
        (!requirements->present || out_queue_info->present_family_index != -1) &&
        (!requirements->compute || out_queue_info->compute_family_index != -1) &&
        (!requirements->transfer || out_queue_info->transfer_family_index != -1)) {
        LOG_INFO("Device meets queue requirements");
        LOG_TRACE("Graphics family index: %i", out_queue_info->graphics_family_index);
        LOG_TRACE("Present family index: %i", out_queue_info->present_family_index);
        LOG_TRACE("Transfer family index: %i", out_queue_info->transfer_family_index);
        LOG_TRACE("Compute family index: %i", out_queue_info->compute_family_index);

        vulkan_device_query_swapchain_support(device, surface, out_swapchain_support);
        if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
            if (out_swapchain_support->formats) {
                m_free(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
            }
            if (out_swapchain_support->present_modes) {
                m_free(out_swapchain_support->present_modes, sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count, MEMORY_TAG_RENDERER);
            }
            LOG_INFO("Required swapchain support not present, skipping device");
            return FALSE;
        }

        if (requirements->device_extension_names) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, 0));
            if (available_extension_count != 0) {
                available_extensions = m_allocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, available_extensions));

                u32 required_extension_count = darray_get_length(requirements->device_extension_names);
                for (u32 i = 0; i < required_extension_count; ++i) {
                    b8 found = FALSE;
                    for (u32 j = 0; j < available_extension_count; ++j) {
                        if (compare_strings(requirements->device_extension_names[i], available_extensions[j].extensionName)) {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                        LOG_INFO("Required extension not found: '%s', skipping device.", requirements->device_extension_names[i]);
                        m_free(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }
            }
            m_free(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
        }

        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            LOG_INFO("Device does not support sampler anisotropy, skipping.");
            return FALSE;
        }

        return TRUE;
    }
    return FALSE;
}

b8 select_physical_device(vulkan_context* context) {
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
    if (physical_device_count == 0) {
        LOG_FATAL("No devices support vulkan");
        return FALSE;
    }

    VkPhysicalDevice physical_devices[physical_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        //TODO: requirements should be driven by engine config
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        //if compute
        //requirements.compute = TRUE;
        requirements.sampler_anisotropy = TRUE;
        requirements.discrete_gpu = TRUE;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info = {};
        b8 result = physical_device_meets_requirements(
            physical_devices[i], 
            context->surface, 
            &properties, 
            &features, 
            &requirements, 
            &queue_info, 
            &context->device.swapchain_support);

        if (result) {
            LOG_INFO("Selected device: '%s'.", properties.deviceName);
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    LOG_INFO("GPU type is unknown");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    LOG_INFO("GPU type is integrated");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    LOG_INFO("GPU type is discrete");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    LOG_INFO("GPU type is virtual");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    LOG_INFO("GPU type is CPU");
                    break;
            }

            LOG_INFO("GPU Driver version: %d.%d.%d", VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));
            LOG_INFO("Vulkan API version: %d.%d.%d", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
            
            //memory info
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    LOG_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
                } else {
                    LOG_INFO("Shared system memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
            //NOTE: compute here when needed
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }
    if (!context->device.physical_device) {
        LOG_ERROR("No physical devices meets requirements");
        return FALSE;
    }
    LOG_INFO("Physical device selected");
    return TRUE;
}

b8 create_vulkan_device(vulkan_context* context) {
    if (!select_physical_device(context)) {
        return FALSE;
    }
    LOG_INFO("Creating logical device");

    //NOTE: do not create additional queues for shared indices
    //based on the queue indices we gathered from select physical device, we determine the overlap for how many queue families we need
    b8 present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
    b8 transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
    u32 index_count = 1;
    if (!present_shares_graphics_queue) {
        index_count++;
    }
    if (!transfer_shares_graphics_queue) {
        index_count++;
    }

    //now with index count determined, size the queue indices array and fill it in
    u32 indices[index_count];
    u8 index = 0;
    indices[index++] = context->device.graphics_queue_index;
    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.present_queue_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transfer_queue_index;
    }
    
    //create and fill out queue create info structs. think of queues as workers, we set up 2 graphics queues to render a frame alongside the next
    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;   //HACK: this is inflexible for now hardcoded to 1
        if (indices[i] == context->device.graphics_queue_index) {
            queue_create_infos[i].queueCount = 2;
        }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    //TODO: configurable
    //request device features
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE; //demand anisotropy

    //fill out device info and make logical device
    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    device_create_info.enabledLayerCount = 0; //depreciated, pass nothing
    device_create_info.ppEnabledLayerNames = 0; //depreciated
    
    VK_CHECK(vkCreateDevice(
        context->device.physical_device, 
        &device_create_info, 
        context->allocator, 
        &context->device.logical_device));
    LOG_INFO("Logical device created");

    //get handles to queues
    vkGetDeviceQueue(context->device.logical_device, context->device.graphics_queue_index, 0, &context->device.graphics_queue);
    vkGetDeviceQueue(context->device.logical_device, context->device.present_queue_index, 0, &context->device.present_queue);
    vkGetDeviceQueue(context->device.logical_device, context->device.transfer_queue_index, 0, &context->device.transfer_queue);
    LOG_INFO("Queues obtained");

    return TRUE;
}

void destroy_vulkan_device(vulkan_context* context) {

    //release queue handles
    context->device.graphics_queue = 0;
    context->device.present_queue = 0;
    context->device.transfer_queue = 0;

    //destroy logical device
    LOG_INFO("Destroying logical device");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = 0;
    }
    
    //destroy physical device
    LOG_INFO("Releasing physical device resources");
    context->device.physical_device = 0;
    if (context->device.swapchain_support.formats) {
        m_free(context->device.swapchain_support.formats, sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count, MEMORY_TAG_RENDERER);
        context->device.swapchain_support.formats = 0;
        context->device.swapchain_support.format_count = 0;
    }
    if (context->device.swapchain_support.present_modes) {
        m_free(context->device.swapchain_support.present_modes, sizeof(VkPresentModeKHR) * context->device.swapchain_support.present_mode_count, MEMORY_TAG_RENDERER);
        context->device.swapchain_support.present_modes = 0;
        context->device.swapchain_support.present_mode_count = 0;
    }
    m_set_memory(&context->device.swapchain_support.capabilities, sizeof(context->device.swapchain_support.capabilities), 0);
    context->device.graphics_queue_index = -1;
    context->device.present_queue_index = -1;
    context->device.transfer_queue_index = -1;
}
