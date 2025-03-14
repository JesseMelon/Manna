#include "vulkan_backend.h"
#include "core/application.h"
#include "core/memory.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_fence.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_renderpass.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "vulkan_types.h"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/mstring.h"
#include "containers/darray.h"
#include "platform/platform.h"

static vulkan_context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

//internal logging callback function to display vulkan messages
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

/**
 * @brief Creates a command buffer for each swapchain image for parrallel workloads
 *
 * @param[in] backend [TODO:description]
 */
void create_command_buffers(renderer_backend* backend) {
    if (!context.graphics_command_buffers) {
        context.graphics_command_buffers = darray_reserve(vulkan_command_buffer, context.swapchain.image_count);
        for (u32 i = 0; i < context.swapchain.image_count; ++i) {
            m_set_memory(&context.graphics_command_buffers[i], 0, sizeof(vulkan_command_buffer));
        }
    }
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.graphics_command_buffers[i].handle) {
            free_vulkan_command_buffer(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
        }
        m_set_memory(&context.graphics_command_buffers[i], 0, sizeof(vulkan_command_buffer));
        allocate_vulkan_command_buffer(&context, context.device.graphics_command_pool, TRUE, &context.graphics_command_buffers[i]);
    }

    LOG_INFO("Vulkan command buffers created");
}

void regenerate_framebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass) {
    for (u32 i = 0; i < swapchain->image_count; ++i) {
        //TODO: make dynamic
        u32 attachment_count = 2;
        VkImageView attachments[] = {swapchain->views[i], swapchain->depth_attachment.view};
        create_vulkan_framebuffer(&context, renderpass, context.framebuffer_width, context.framebuffer_height, attachment_count, attachments, &context.swapchain.framebuffers[i]);
    }
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (type_filter & (i << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }
    LOG_WARN("Unable to find memory type");
    return -1;
}

b8 init_vulkan_renderer_backend(renderer_backend *backend, const char *application_name, struct platform_state *platform_state) {
    
    context.find_memory_index = find_memory_index;

    //TODO: allocator
    context.allocator = 0;
    
    //catches and fixes 0 framebuffer size
    application_get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;
    

    //fill out application info struct
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
    app_info.pEngineName = "Manna";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    //fill out instance creation info struct using application info struct
    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    //gather required extensions in darray
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);   //generic vulkan extension, defines surfaces, always required
    vulkan_platform_get_required_extension_names(&required_extensions);        //get platform specific extension(s)
#if defined (DEBUG)
    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);   //get debug utility extensions if debugging
    LOG_DEBUG("Required extensions:");
    u32 length = darray_get_length(required_extensions);
    for (u32 i = 0; i < length; ++i) {
        LOG_DEBUG(required_extensions[i]);
    }
#endif

    //add extension names to create info
    create_info.enabledExtensionCount = darray_get_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

    //if validating, get a list of required validation layer names and enforce they exist
#if defined (DEBUG)
    LOG_INFO("Validation layers enabled. Enumerating");

    required_validation_layer_names = darray_create(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");   //first validation layer
    required_validation_layer_count = darray_get_length(required_validation_layer_names);

    //creates a list of all expected validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    //verify all expected layers are available
    for (u32 i = 0; i < required_validation_layer_count; ++i) {
        LOG_INFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (compare_strings(required_validation_layer_names[i], available_layers[j].layerName)) {
                found = TRUE;
                LOG_INFO("Found");
                break;
            }
        }
        if (!found) {
            LOG_FATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return FALSE;
        }
    }
    LOG_INFO("All required validation layers are present");
#endif
    
    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    LOG_INFO("Vulkan instance created");

#if defined (DEBUG)
    LOG_DEBUG("Creating vulkan debugger");
    u32 message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // |
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = message_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = 0;
    
    //load a function pointer to our debugger function (context.debug_messenger)
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    M_ASSERT(func, "failed to create debug messenger");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    LOG_DEBUG("Vulkan debugger created");
#endif
    
    //let platform layer handle surface creation
    LOG_DEBUG("Creating vulkan surface");
    if (!platform_create_vulkan_surface(platform_state, &context)) {
        LOG_ERROR("Failed to create platform surface");
        return FALSE;
    }
    LOG_DEBUG("Vulkan surface created");
    
    if (!create_vulkan_device(&context)) {
        LOG_ERROR("Failed to create device");
        return FALSE;
    }

    create_vulkan_swapchain(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain);

    create_vulkan_renderpass(&context, &context.main_renderpass, 0, 0, context.framebuffer_width, context.framebuffer_height, 0.0f, 0.3f, 0.0f, 1.0f, 1.0f, 0);

    context.swapchain.framebuffers = darray_reserve(vulkan_framebuffer, context.swapchain.image_count);
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    //create sync objects
    context.image_available_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queue_complete_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.in_flight_fences = darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight);

    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.image_available_semaphores[i]);
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.queue_complete_semaphores[i]);

        create_vulkan_fence(&context, TRUE, &context.in_flight_fences[i]);
    }
    
    //in flight fences array is reserved, but not yet used. fences do not yet exist, so the array of pointers to them is merely created in advance
    context.images_in_flight = darray_reserve(vulkan_fence, context.swapchain.image_count);
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_flight[i] = 0;
    }

    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void shutdown_vulkan_renderer_backend(renderer_backend *backend) {

    //wait for device to finish up what is currently doing
    vkDeviceWaitIdle(context.device.logical_device);

    //Sync objects
    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        if (context.image_available_semaphores[i]) {
            vkDestroySemaphore(context.device.logical_device, context.image_available_semaphores[i], context.allocator);
            context.image_available_semaphores[i] = 0;
        }
        if (context.queue_complete_semaphores[i]) {
            vkDestroySemaphore(context.device.logical_device, context.queue_complete_semaphores[i], context.allocator);
            context.queue_complete_semaphores[i] = 0;
        }
        destroy_vulkan_fence(&context, &context.in_flight_fences[i]);
    }
    darray_destroy(context.image_available_semaphores);
    context.image_available_semaphores = 0;

    darray_destroy(context.queue_complete_semaphores);
    context.queue_complete_semaphores = 0;

    darray_destroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    darray_destroy(context.images_in_flight);
    context.images_in_flight = 0;

    //command buffers
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.graphics_command_buffers[i].handle) {
            free_vulkan_command_buffer(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
            context.graphics_command_buffers[i].handle = 0;
        }
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;
    
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        destroy_vulkan_framebuffer(&context, &context.swapchain.framebuffers[i]);
    }

    destroy_vulkan_renderpass(&context, &context.main_renderpass);

    destroy_vulkan_swapchain(&context, &context.swapchain);
    
    LOG_DEBUG("Destroying vulkan device");
    destroy_vulkan_device(&context);

    LOG_DEBUG("Destroying vulkan surface");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    LOG_DEBUG("Destroying vulkan debugger");
    if(context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }

    LOG_DEBUG("Destroying vulkan instance");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_on_resize(renderer_backend *backend, u16 width, u16 height) {

}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time) {
    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time) {
    return TRUE;
}
