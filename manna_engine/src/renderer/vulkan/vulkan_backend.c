#include "vulkan_backend.h"
#include "core/application.h"
#include "math/math_types.h"
#include "memory/memory.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_command_buffer.h"
#include "renderer/vulkan/vulkan_fence.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_renderpass.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "vulkan_types.h"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/mstring.h"
#include "containers/darray.h"

//shaders
#include "shaders/vulkan_object_shader.h"

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

b8 recreate_swapchain(renderer_backend* backend) {
    //check to see if already recreating
    if (context.recreating_swapchain) {
        LOG_DEBUG("recreating_swapchain called while already recreating");
        return FALSE;
    }

    //detect if window is too small
    if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        LOG_DEBUG("recreate_swapchain called when window is < 1 in a dimension");
        return FALSE;
    }

    context.recreating_swapchain = TRUE;

    //wait for things to finish
    vkDeviceWaitIdle(context.device.logical_device);
    
    //clear images of wrong dimension
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_flight[i] = 0;
    }

    //requery swapchain support
    vulkan_device_query_swapchain_support(context.device.physical_device, context.surface, &context.device.swapchain_support);
    vulkan_device_detect_depth_format(&context.device);

    recreate_vulkan_swapchain(&context, cached_framebuffer_width, cached_framebuffer_height, &context.swapchain);
    
    //sync framebuffer with cached sizes
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    context.framebuffer_size_last_generation = context.framebuffer_size_generation;

    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        free_vulkan_command_buffer(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        destroy_vulkan_framebuffer(&context, &context.swapchain.framebuffers[i]);
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    //signal done
    context.recreating_swapchain = FALSE;

    return TRUE;
}

//why the extra step? because device local memory is used in vkmemorypropertyflagbits (set in create_buffers) this is because device local memory is
//faster than host coherent.
//TODO: totally refactor this and move it somewhere
void upload_data_range(vulkan_context* context, VkCommandPool pool, VkFence fence, VkQueue queue, vulkan_buffer* buffer, u64 offset, u64 size, void* data) {
    LOG_DEBUG("Uploading data range");
    //create host visible staging buffer to upload to. Mark as source of transfer
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    create_vulkan_buffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    //load data into staging buffer
    load_vulkan_buffer_data(context, &staging, 0, size, 0, data);

    //copy from staging buffer to device local buffer
    copy_vulkan_buffer(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);

    //clean up staging buffer
    destroy_vulkan_buffer(context, &staging);
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

static b8 create_buffers(vulkan_context* context) {
    LOG_DEBUG("Creating Buffers");
    VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    const u64 vertex_buffer_size = sizeof(vertex) * 1024 * 1024; //this is overkill?
    if (!create_vulkan_buffer(context,
                              vertex_buffer_size,
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              memory_property_flags,
                              true,
                              &context->object_vertex_buffer)) {
        LOG_ERROR("Error creating vertex buffer");
        return false;
    }
    context->geometry_vertex_offset = 0;

    const u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    if (!create_vulkan_buffer(context,
                              index_buffer_size,
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              memory_property_flags,
                              true,
                              &context->object_index_buffer)) {
        LOG_ERROR("Error creating index buffer");
        return false;
    }
    context->geometry_index_offset = 0;

    return true;
}

b8 init_vulkan_renderer_backend(renderer_backend* backend, const char *application_name) {
    
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
    if (!platform_create_vulkan_surface(&context)) {
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

    if (!create_vulkan_object_shader(&context, &context.object_shader)) {
        LOG_ERROR("Error loading built-in shader");
        return false;
    }

    create_buffers(&context);

    //HACK: Temporary test code
#define TEMP_VERTEX_COUNT 3
    vertex verts[TEMP_VERTEX_COUNT];
    m_set_memory(verts, 0, sizeof(vertex) * TEMP_VERTEX_COUNT);

    verts[0].position.x = 0.0;
    verts[0].position.y = -0.5;
    verts[0].position.z = 0;
    verts[1].position.x = 0.5;
    verts[1].position.y = 0.5;
    verts[1].position.z = 0;
    verts[2].position.x = 0;
    verts[2].position.y = 0.5;
    verts[2].position.z = 0;

#define TEMP_INDEX_COUNT 3
    u32 indices[TEMP_INDEX_COUNT] = {0, 1, 2};

    upload_data_range(&context, context.device.graphics_command_pool, 0, context.device.graphics_queue, &context.object_vertex_buffer, 0, sizeof(vertex) * TEMP_VERTEX_COUNT, verts);
    upload_data_range(&context, context.device.graphics_command_pool, 0, context.device.graphics_queue, &context.object_index_buffer, 0, sizeof(u32) * TEMP_INDEX_COUNT, indices);
    //end of test section


    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void shutdown_vulkan_renderer_backend(renderer_backend *backend) {

    //wait for device to finish up what is currently doing
    vkDeviceWaitIdle(context.device.logical_device);

    destroy_vulkan_buffer(&context, &context.object_vertex_buffer);
    destroy_vulkan_buffer(&context, &context.object_index_buffer);

    destroy_vulkan_object_shader(&context, &context.object_shader);

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
#if defined (DEBUG)
    LOG_DEBUG("Destroying vulkan debugger");
    if(context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }
#endif
    LOG_DEBUG("Destroying vulkan instance");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_on_resize(renderer_backend *backend, u16 width, u16 height) {
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_size_generation++;

    LOG_INFO("Vulkan renderer backend resized: w/h/gen: %i/%i/%llu", width, height, context.framebuffer_size_generation);
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time) {

    if (context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(context.device.logical_device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: %s", vulkan_result_string(result, TRUE));
            return FALSE;
        }
        LOG_INFO("Recreating swapchain.");
        return FALSE;
    }

    if (context.framebuffer_size_generation != context.framebuffer_size_last_generation) {
        VkResult result = vkDeviceWaitIdle(context.device.logical_device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: %s", vulkan_result_string(result, TRUE));
            return FALSE;
        }
        if (!recreate_swapchain(backend)) {
            return FALSE;
        }

        LOG_INFO("Resized");
        return FALSE;
    }

    //wait for current frame to complete
    if (!wait_for_vulkan_fence(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX)) {
        LOG_WARN("In-flight fence wait failure");
        return FALSE;
    }

    //acquire next image from swapchain
    if (!vulkan_swapchain_acquire_next_image_index(&context, &context.swapchain, UINT64_MAX, context.image_available_semaphores[context.current_frame], 0, &context.image_index)) {
        return FALSE;   
    }

    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    reset_vulkan_command_buffer(command_buffer);
    begin_vulkan_command_buffer(command_buffer, FALSE, FALSE, FALSE);

    //dynamic state, treating bottom left as 0,0 and depth from 0 to 1 consistent with opengl
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor to size of screen. could make smaller for demonstration 
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    begin_vulkan_renderpass(command_buffer, &context.main_renderpass, context.swapchain.framebuffers[context.image_index].handle);

    //HACK: temporary test code
    //use shader
    use_vulkan_object_shader(&context, &context.object_shader);

    //bind vertex buffer at offset
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, (VkDeviceSize*)offsets);

    //bind index buffer at offset
    vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

    //draw call
    vkCmdDrawIndexed(command_buffer->handle, TEMP_INDEX_COUNT, 1, 0, 0, 0);
    //end of test section

    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time) {

    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];

    end_vulkan_renderpass(command_buffer, &context.main_renderpass);

    end_vulkan_command_buffer(command_buffer);

    if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) {
        wait_for_vulkan_fence(&context, context.images_in_flight[context.image_index], UINT64_MAX);
    }

    //mark the image fence as in use
    context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

    //reset fence for the next frame
    reset_vulkan_fence(&context, &context.in_flight_fences[context.current_frame]);

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;
    //semaphores to be signalled when queue completes
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame];
    //wait semaphore ensures operation cannot begin until the image is available
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];
    //each semaphore waits on the corresponding pipeline stage to complete.
    //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
    //writes from executing until the semaphore signals
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(context.device.graphics_queue, 1, &submit_info, context.in_flight_fences[context.current_frame].handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("vkQueueSubmit failed with result: %s", vulkan_result_string(result, TRUE));
        return FALSE;
    }

    submit_vulkan_command_buffer(command_buffer);
    //queue submission done

    //present
    vulkan_swapchain_present_image(&context, &context.swapchain, context.device.graphics_queue, context.device.present_queue, context.queue_complete_semaphores[context.current_frame], context.image_index);

    return TRUE;
}
