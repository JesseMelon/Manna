#include "vulkan_backend.h"
#include "core/asserts.h"
#include "core/logger.h"
#include "defines.h"
#include "vulkan_device.h"
#include "vulkan_types.h"
#include "core/mstring.h"
#include "containers/darray.h"
#include "vulkan_platform.h"

static vulkan_context context;

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

b8 init_vulkan_renderer_backend(renderer_backend *backend, const char *application_name, struct platform_state *platform_state) {
    
    //TODO: allocator
    context.allocator = 0;

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
    
    //add validation layers to create info
    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    //create instance using create info
    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    LOG_INFO("Vulkan instance created");

#if defined (DEBUG)
    LOG_DEBUG("Creating vulkan debugger");
    u32 message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // |
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    //fill out debugger creation info 
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
    
    //create device from context
    if (!create_vulkan_device(&context)) {
        LOG_ERROR("Failed to create device");
        return FALSE;
    }

    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void shutdown_vulkan_renderer_backend(renderer_backend *backend) {
    
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
